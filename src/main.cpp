#include <iostream>
#include <memory>
#include <cmath>
#include <utility>
#include <algorithm>
#include <vector>
#include <lodepng.h>

float kernel(const float r, const float h) {
	return std::exp(-r*r) - std::exp(-h*h);
}

constexpr std::size_t num_top_colors = 5;
constexpr std::size_t default_top_colors_min_distance = 30;

int main(int argc, char** argv) {
	if(argc <= 1) {
		return 1;
	}
	std::string input_filename = argv[1];

	std::size_t top_colors_min_distance = default_top_colors_min_distance;
	if(argc >= 3) {
		top_colors_min_distance = std::stoul(argv[2]);
	}

	unsigned width, height;
	std::vector<unsigned char> image;

	const auto error = lodepng::decode(image, width, height, input_filename);

	std::printf("# Image info\n"
				"- Size : %u, %u\n"
				, width, height
				);

	constexpr std::size_t color_dim = 256;
	constexpr std::size_t h = 5;
	constexpr std::size_t color_space_size = color_dim * color_dim * color_dim;

	float kernel_table[h * h + 1];
	for(std::size_t i = 0; i <= h * h; i++) {
		kernel_table[i] = kernel(std::sqrt(static_cast<float>(i)), static_cast<float>(h));
	}

	std::unique_ptr<float[]> color_space(new float [color_space_size]);
	for(std::size_t i = 0; i < color_space_size; i++) {
		color_space.get()[i] = 0.0f;
	}

	//clustering
#pragma omp parallel for
	for(std::size_t x = 0; x < width; x++) {
		for(std::size_t y = 0; y < height; y++) {
			const auto index = (x * height + y) * 4;
			const auto R = static_cast<unsigned>(image[index + 0]);
			const auto G = static_cast<unsigned>(image[index + 1]);
			const auto B = static_cast<unsigned>(image[index + 2]);

			for(int r = R - h; r <= R + h; r++) {
				for(int g = G - h; g <= G + h; g++) {
					for(int b = B - h; b <= B + h; b++) {
						if((r >= 0 && 255 >= r) &&
							(g >= 0 && 255 >= g) &&
							(b >= 0 && 255 >= b)) {
							const auto dr = r - R;
							const auto dg = g - G;
							const auto db = b - B;
							const auto r2 = dr * dr + dg * dg + db * db;
							if(r2 <= h * h) {
#pragma omp atomic
								color_space.get()[r * color_dim * color_dim + g * color_dim + b] += kernel_table[r2];
							}
						}
					}
				}
			}
		}
	}

	std::vector<std::pair<std::size_t, float>> color_candidate;
	for(std::size_t i = 0; i < color_space_size; i++) {
		if(color_space.get()[i] / (width * height) > kernel_table[0] * 0.00005f) {
			color_candidate.push_back(std::make_pair(i, color_space.get()[i]));
		}
	}

	std::sort(color_candidate.begin(), color_candidate.end(), [](const std::pair<std::size_t, float>& a, const std::pair<std::size_t, float>& b){return a.second > b.second;});

	std::printf("# Top colors candidate (%lu)\n", color_candidate.size());
	std::vector<std::pair<std::size_t, float>> top_colors;
	for(auto &color : color_candidate) {
		const int r = color.first / (color_dim * color_dim);
		const int g = (color.first / color_dim) % color_dim;
		const int b = color.first % color_dim;

		bool is_addable = true;
		for(auto added_color : top_colors) {
			const int ar = added_color.first / (color_dim * color_dim);
			const int ag = (added_color.first / color_dim) % color_dim;
			const int ab = added_color.first % color_dim;

			const auto dr = ar - r;
			const auto dg = ag - g;
			const auto db = ab - b;

			//std::printf("[%5e] : (%3d, %3d, %3d) #%02x%02x%02x\n", color.second, r, g, b, r, g, b);

			if(dr * dr + dg * dg + db * db < top_colors_min_distance * top_colors_min_distance) {
				is_addable = false;
			}

		}
		if(is_addable) {
			top_colors.push_back(color);
		}
	}

	std::printf("# Sorted colors (%lu)\n", top_colors.size());
	for(const auto &color : top_colors) {
		const int r = color.first / (color_dim * color_dim);
		const int g = (color.first / color_dim) % color_dim;
		const int b = color.first % color_dim;

		std::printf("<span style=\"color:#%02x%02x%02x\">[%5e] : (%3d, %3d, %3d) #%02x%02x%02x</span><br>\n", r, g, b, color.second, r, g, b, r, g, b);
	}
}
