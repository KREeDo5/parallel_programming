#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <cmath>
#include <algorithm>
#include <string>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include <Windows.h>

#pragma pack(push, 1) // отключение выравнивания структуры
struct BMPHeader {
    uint16_t file_type{ 0x4D42 };
    uint32_t file_size{ 0 };
    uint16_t reserved1{ 0 };
    uint16_t reserved2{ 0 };
    uint32_t offset_data{ 0 };
};

struct BMPInfoHeader {
    uint32_t size{ 0 };
    int32_t width{ 0 };
    int32_t height{ 0 };
    uint16_t planes{ 1 };
    uint16_t bit_count{ 0 };
    uint32_t compression{ 0 };
    uint32_t size_image{ 0 };
    int32_t x_pixels_per_meter{ 0 };
    int32_t y_pixels_per_meter{ 0 };
    uint32_t colors_used{ 0 };
    uint32_t colors_important{ 0 };
};

struct BMPColorHeader {
    uint32_t red_mask{ 0x00ff0000 };
    uint32_t green_mask{ 0x0000ff00 };
    uint32_t blue_mask{ 0x000000ff };
    uint32_t alpha_mask{ 0xff000000 };
    uint32_t color_space_type{ 0x73524742 };
    uint32_t unused[16]{ 0 };
};
#pragma pack(pop)

struct BMPImage {
    BMPHeader file_header;
    BMPInfoHeader bmp_info_header;
    BMPColorHeader bmp_color_header;
    std::vector<uint8_t> data;

    BMPImage() = default;

    BMPImage(const std::string& file_name) {
        read(file_name);
    }

    void read(const std::string& file_name) {
        std::ifstream inp(file_name, std::ios::binary);
        if (!inp) {
            throw std::runtime_error("Ошибка при открытии файла " + file_name);
        }

        inp.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
        if (file_header.file_type != 0x4D42) {
            throw std::runtime_error("Неверный формат файла. Ожидался BMP.");
        }

        inp.read(reinterpret_cast<char*>(&bmp_info_header), sizeof(bmp_info_header));

        if (bmp_info_header.bit_count == 32) {
            inp.read(reinterpret_cast<char*>(&bmp_color_header), sizeof(bmp_color_header));
        }

        inp.seekg(file_header.offset_data, inp.beg);

        size_t row_padded = (bmp_info_header.width * bmp_info_header.bit_count / 8 + 3) & (~3);
        data.resize(row_padded * bmp_info_header.height);
        inp.read(reinterpret_cast<char*>(data.data()), data.size());
    }

    void write(const std::string& file_name) const {
        std::ofstream of(file_name, std::ios::binary);
        if (!of) {
            throw std::runtime_error("Ошибка при записи файла " + file_name);
        }

        of.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
        of.write(reinterpret_cast<const char*>(&bmp_info_header), sizeof(bmp_info_header));
        if (bmp_info_header.bit_count == 32) {
            of.write(reinterpret_cast<const char*>(&bmp_color_header), sizeof(bmp_color_header));
        }

        of.write(reinterpret_cast<const char*>(data.data()), data.size());
    }
};

static void blur(const BMPImage& input, BMPImage& output, int start_row, int end_row) {
    int width = input.bmp_info_header.width;
    int height = input.bmp_info_header.height;
    int bytes_per_pixel = input.bmp_info_header.bit_count / 8;

    size_t row_padded = (width * bytes_per_pixel + 3) & (~3);

    for (int y = start_row; y < end_row; ++y) {
        for (int x = 0; x < width; ++x) {
            int sum_r = 0, sum_g = 0, sum_b = 0;
            int count = 0;

            // Блюр размера 3x3
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int nx = std::clamp(x + kx, 0, width - 1);
                    int ny = std::clamp(y + ky, 0, height - 1);

                    size_t index = ny * row_padded + nx * bytes_per_pixel;
                    uint8_t b = input.data[index];
                    uint8_t g = input.data[index + 1];
                    uint8_t r = input.data[index + 2];

                    sum_r += r;
                    sum_g += g;
                    sum_b += b;
                    count++;
                }
            }

            // Запись среднего значения
            size_t out_index = y * row_padded + x * bytes_per_pixel;
            output.data[out_index] = static_cast<uint8_t>(sum_b / count);
            output.data[out_index + 1] = static_cast<uint8_t>(sum_g / count);
            output.data[out_index + 2] = static_cast<uint8_t>(sum_r / count);
            if (bytes_per_pixel == 4) {
                output.data[out_index + 3] = input.data[out_index + 3];
            }
        }
    }
}

static void blur_multithread(const BMPImage& input, BMPImage& output, int threads_count) {
    std::vector<std::thread> threads;
    int rows_per_thread = input.bmp_info_header.height / threads_count;
    int remaining_rows = input.bmp_info_header.height % threads_count;

    int current_start = 0;

    for (int i = 0; i < threads_count; ++i) {
        int current_end = current_start + rows_per_thread;
        if (i == threads_count - 1) {
            current_end += remaining_rows; // Последний поток обрабатывает оставшиеся строки
        }

        threads.emplace_back(blur, std::ref(input), std::ref(output), current_start, current_end);
        current_start = current_end;
    }

    for (auto& t : threads) {
        t.join();
    }
}

int main(int argc, char* argv[]) {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    if (argc != 5) {
        std::cerr << "Использование: " << argv[0] << " <входной BMP файл> <выходной BMP файл> <количество потоков> <количество ядер>" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    int threads_count = std::stoi(argv[3]);
    int cores_count = std::stoi(argv[4]); // для ядер

    try {
        BMPImage input(input_file);
        BMPImage output = input;

        blur_multithread(input, output, threads_count);

        output.write(output_file);
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
