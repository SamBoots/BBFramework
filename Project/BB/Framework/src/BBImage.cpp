#include "BBImage.hpp"
#include "OS/Program.h"
#include "BBThreadScheduler.hpp"

#include "Math.inl"
#include <immintrin.h>

#define PACK_RGBA(r, g, b, a) (uint32_t)(r<<24|g<<16|b<<8|a)

using namespace BB;

namespace BMP
{
	constexpr uint16_t BMP_IMAGE_FILE_TYPE = 0x4D42;
	constexpr uint32_t BMP_RGBA_COLOR_SPACE = 0x73524742;
#pragma pack(push, 1)
	struct File
	{
		uint16_t file_type; //2
		uint32_t file_size; //6
		uint16_t reserved1; //8
		uint16_t reserved2; //10
		uint32_t offset_data; //14
	} m_file;
	struct Header
	{
		uint32_t size; //4
		int32_t width; //8
		int32_t height; //12
		uint16_t planes; //14
		uint16_t bit_count; //16
		uint32_t compression; //20
		uint32_t size_image; //24
		int32_t x_pixels_per_meter; //28
		int32_t y_pixels_per_meter; //32
		uint32_t colors_used; //36
		uint32_t colors_important; //40
	} m_header;
	struct ColorHeader
	{
		uint32_t red_mask; //4
		uint32_t green_mask; //8 
		uint32_t blue_mask; //12
		uint32_t alpha_mask; //16
		uint32_t color_space_type; //20
		uint32_t unused[16]; //84
	} m_color_header;
#pragma pack(pop)


	static bool isRGBA(const ColorHeader& a_color_header)
	{
		ColorHeader rgb;
		rgb.red_mask = 0x00ff0000;
		rgb.green_mask = 0x0000ff00;
		rgb.blue_mask = 0x000000ff;
		rgb.alpha_mask = 0xff000000;
		rgb.color_space_type = BMP_RGBA_COLOR_SPACE;

		if (a_color_header.color_space_type != rgb.color_space_type)
		{
			return false;
		}

		if (a_color_header.red_mask != rgb.red_mask ||
			a_color_header.green_mask != rgb.green_mask ||
			a_color_header.blue_mask != rgb.blue_mask ||
			a_color_header.alpha_mask != rgb.alpha_mask)
		{
			BB_ASSERT(false, "BMP color header is not RGBA while the color space type is assigned as RGBA!");
			return false;
		}
		return true;
	}
}


BBImage::BBImage(Allocator a_allocator, const char* a_file_path)
{
	LoadBMP(a_allocator, a_file_path);
}
BBImage::BBImage(Allocator a_allocator, const uint32_t a_width, const uint32_t a_height, const bool has_alpha)
{
	//todo
}

BBImage::BBImage(Allocator a_allocator, const BBImage& a_image)
{
	m_width = a_image.m_width;
	m_height = a_image.m_height;
	m_row_stride = a_image.m_row_stride;
	m_bit_count = a_image.m_bit_count;
	m_red_mask = a_image.m_red_mask;
	m_green_mask = a_image.m_green_mask;
	m_blue_mask = a_image.m_blue_mask;
	m_alpha_mask = a_image.m_alpha_mask;

	m_data_size = a_image.m_data_size;
	m_data = BBalloc(a_allocator, m_data_size);
	memcpy(m_data, a_image.m_data, m_data_size);
}

BBImage& BBImage::operator=(const BBImage& a_Rhs)
{
	BB_ASSERT(a_Rhs.m_width == a_Rhs.m_width && m_height == a_Rhs.m_height, "trying to do a copy operation on a image that is not the same width and height!");
	m_width = a_Rhs.m_width;
	m_height = a_Rhs.m_height;
	m_row_stride = a_Rhs.m_row_stride;
	m_bit_count = a_Rhs.m_bit_count;
	m_red_mask = a_Rhs.m_red_mask;
	m_green_mask = a_Rhs.m_green_mask;
	m_blue_mask = a_Rhs.m_blue_mask;
	m_alpha_mask = a_Rhs.m_alpha_mask;

	m_data_size = a_Rhs.m_data_size;
	memcpy(m_data, a_Rhs.m_data, m_data_size);
	return *this;
}


struct process_image_part_params
{
	Threads::Barrier* barrier;

	const uint32_t* img_start_old_pixel;
	uint32_t* img_start_new_pixel;
	uint32_t img_height_start;
	uint32_t img_height_end;
	uint32_t img_width;
	uint32_t img_height;

	uint32_t red_mask;
	uint32_t green_mask;
	uint32_t blue_mask;
	uint32_t alpha_mask;

	const float* filter;
	uint32_t filter_width; 
	uint32_t filter_height;
	float factor;
	float bias;
};

void FilterImagePart(void* a_param)
{
	const process_image_part_params& params = *reinterpret_cast<process_image_part_params*>(a_param);

	if (false) //SIMD
	{
		const __m128 simd_factor = _mm_set_ps1(params.factor);
		const __m128 simd_bias = _mm_set_ps1(params.bias);
		const __m128 simd_rgba_min = _mm_setzero_ps();
		const __m128 simd_rgba_max = _mm_set_ps1(255.f);

		for (int y = params.img_height_start; y < params.img_height_end; y++)
			for (int x = 0; x < params.img_width; x++)
			{
				__m128 simd_rgba = _mm_setzero_ps();

				for (int filterY = 0; filterY < params.filter_height; filterY++)
					for (int filterX = 0; filterX < params.filter_width; filterX++)
					{
						const int imageX = (x - params.filter_width / 2 + filterX + params.img_width) % params.img_width;
						const int imageY = (y - params.filter_height / 2 + filterY + params.img_height) % params.img_height;
						const uint32_t pixel = reinterpret_cast<const uint32_t*>(params.img_start_old_pixel)[imageY * params.img_width + imageX];

						const __m128 simd_filter = _mm_set_ps1(params.filter[filterY * params.filter_width + filterX]);
						const __m128 rgba_mod = _mm_set_ps(
							(pixel & params.blue_mask) >> 0,
							(pixel & params.green_mask) >> 8,
							(pixel & params.red_mask) >> 16,
							(pixel & params.alpha_mask) >> 24);

						simd_rgba = _mm_add_ps(simd_rgba, _mm_mul_ps(rgba_mod, simd_filter));
					}
				simd_rgba = _mm_mul_ps(simd_factor, simd_rgba);
				_mm_min_ps(_mm_max_ps(_mm_add_ps(simd_rgba, simd_bias), simd_rgba_min), simd_rgba_max);

				const uint32_t packed = PACK_RGBA(
					static_cast<uint32_t>(simd_rgba.m128_f32[0]),
					static_cast<uint32_t>(simd_rgba.m128_f32[1]),
					static_cast<uint32_t>(simd_rgba.m128_f32[2]),
					static_cast<uint32_t>(simd_rgba.m128_f32[3]));
				reinterpret_cast<uint32_t*>(params.img_start_new_pixel)[y * params.img_width + x] = packed;
			}
	}
	else //non-SIMD
	{
		for (int y = params.img_height_start; y < params.img_height_end; y++)
			for (int x = 0; x < params.img_width; x++)
			{
				float a = 0.f;
				float r = 0.f;
				float g = 0.f;
				float b = 0.f;
				for (int filterY = 0; filterY < params.filter_height; filterY++)
					for (int filterX = 0; filterX < params.filter_width; filterX++)
					{
						const int imageX = (x - params.filter_width / 2 + filterX + params.img_width) % params.img_width;
						const int imageY = (y - params.filter_height / 2 + filterY + params.img_height) % params.img_height;
						const uint32_t pixel = reinterpret_cast<const uint32_t*>(params.img_start_old_pixel)[imageY * params.img_width + imageX];
						const float filter = params.filter[filterY * params.filter_width + filterX];
						a += ((pixel & params.alpha_mask) >> 24) * filter;
						r += ((pixel & params.red_mask) >> 16) * filter;
						g += ((pixel & params.green_mask) >> 8) * filter;
						b += ((pixel & params.blue_mask) >> 0) * filter;
					}

				a = Clampf(params.factor * a + params.bias, 0, 255);
				r = Clampf(params.factor * r + params.bias, 0, 255);
				g = Clampf(params.factor * g + params.bias, 0, 255);
				b = Clampf(params.factor * b + params.bias, 0, 255);

				const uint32_t packed = PACK_RGBA(static_cast<uint32_t>(a), static_cast<uint32_t>(r), static_cast<uint32_t>(g), static_cast<uint32_t>(b));
				reinterpret_cast<uint32_t*>(params.img_start_new_pixel)[y * params.img_width + x] = packed;
			}
	}

	//signal that we are done.
	params.barrier->Signal();
}

void BBImage::FilterImage(Allocator a_temp_allocator, const float* a_filter, const uint32_t a_filter_width, const uint32_t a_filter_height, const float a_factor, const float a_bias, const uint32_t a_thread_count)
{
	void* old_data = BBalloc(a_temp_allocator, m_data_size);
	memcpy(old_data, m_data, m_data_size);

	if (m_bit_count == 32)
	{
		if (a_thread_count > 1)
		{
			const uint32_t pixel_height_per_thread = m_height / a_thread_count;
			const uint32_t worker_threads = a_thread_count - 1;

			//allocate on heap for better memory locality :) maybe.
			Threads::Barrier* thread_barrier = BBnew(a_temp_allocator, Threads::Barrier)(a_thread_count);

			process_image_part_params* params = BBnewArr(a_temp_allocator, a_thread_count, process_image_part_params);
			params[0].barrier = thread_barrier;
			params[0].img_start_new_pixel = reinterpret_cast<uint32_t*>(m_data);
			params[0].img_start_old_pixel = reinterpret_cast<const uint32_t*>(old_data);
			params[0].img_width = m_width;
			params[0].img_height = m_height;

			params[0].alpha_mask = m_alpha_mask;
			params[0].red_mask = m_red_mask;
			params[0].green_mask = m_green_mask;
			params[0].blue_mask = m_blue_mask;

			params[0].filter = a_filter;
			params[0].filter_width = a_filter_width;
			params[0].filter_height = a_filter_height;
			params[0].factor = a_factor;
			params[0].bias = a_bias;

			for (size_t i = 0; i < worker_threads; i++)
			{
				params[i] = params[0];
				params[i].img_height_start = pixel_height_per_thread * i;
				params[i].img_height_end = params[i].img_height_start + pixel_height_per_thread;

				Threads::StartTaskThread(FilterImagePart, &params[i]);
			}

			params[worker_threads] = params[0];
			params[worker_threads].img_height_start = pixel_height_per_thread * worker_threads;
			params[worker_threads].img_height_end = params[worker_threads].img_height_start + pixel_height_per_thread;
			//now send the main thread for some work.
			FilterImagePart(&params[worker_threads]);

			thread_barrier->Wait();
		}
		else if (a_thread_count == 1)
		{
			const __m128 simd_factor = _mm_set_ps1(a_factor);
			const __m128 simd_bias = _mm_set_ps1(a_bias);
			const __m128 simd_rgba_min = _mm_setzero_ps();
			const __m128 simd_rgba_max = _mm_set_ps1(255.f);
			
			for (int y = 0; y < m_height; y++)
				for (int x = 0; x < m_width; x++)
				{
					__m128 simd_rgba = _mm_setzero_ps();

					for (int filterY = 0; filterY < a_filter_height; filterY++)
					for (int filterX = 0; filterX < a_filter_width; filterX++)
					{
						const int imageX = (x - a_filter_width / 2 + filterX + m_width) % m_width;
						const int imageY = (y - a_filter_height / 2 + filterY + m_height) % m_height;
						const uint32_t pixel = reinterpret_cast<uint32_t*>(old_data)[imageY * m_width + imageX];
						
						const __m128 simd_filter = _mm_set_ps1(a_filter[filterY * a_filter_width + filterX]);
						const __m128 rgba_mod = _mm_set_ps(
							(pixel & m_blue_mask) >> 0,
							(pixel & m_green_mask) >> 8,
							(pixel & m_red_mask) >> 16,
							(pixel & m_alpha_mask) >> 24);

						simd_rgba = _mm_add_ps(simd_rgba, _mm_mul_ps(rgba_mod, simd_filter));
					}
					simd_rgba = _mm_mul_ps(simd_factor, simd_rgba);
					_mm_min_ps(_mm_max_ps(_mm_add_ps(simd_rgba, simd_bias), simd_rgba_min), simd_rgba_max);

					const uint32_t packed = PACK_RGBA(
						static_cast<uint32_t>(simd_rgba.m128_f32[0]),
						static_cast<uint32_t>(simd_rgba.m128_f32[1]),
						static_cast<uint32_t>(simd_rgba.m128_f32[2]),
						static_cast<uint32_t>(simd_rgba.m128_f32[3]));
					reinterpret_cast<uint32_t*>(m_data)[y * m_width + x] = packed;
				}
		}
		else //non SIMD
		{
			for (int y = 0; y < m_height; y++)
				for (int x = 0; x < m_width; x++)
				{
					float r = 0.f;
					float g = 0.f;
					float b = 0.f;
					float a = 0.f;

					for (int filterY = 0; filterY < a_filter_height; filterY++)
					for (int filterX = 0; filterX < a_filter_width; filterX++)
					{
						const int imageX = (x - a_filter_width / 2 + filterX + m_width) % m_width;
						const int imageY = (y - a_filter_height / 2 + filterY + m_height) % m_height;
						const uint32_t pixel = reinterpret_cast<uint32_t*>(old_data)[imageY * m_width + imageX];
						const float filter = a_filter[filterY * a_filter_width + filterX];

						b += ((pixel & m_blue_mask) >> 0) * filter;
						g += ((pixel & m_green_mask) >> 8) * filter;
						r += ((pixel & m_red_mask) >> 16) * filter;
						a += ((pixel & m_alpha_mask) >> 24) * filter;
					}

					r = Clampf(a_factor * r + a_bias, 0, 255);
					g = Clampf(a_factor * g + a_bias, 0, 255);
					b = Clampf(a_factor * b + a_bias, 0, 255);
					a = Clampf(a_factor * a + a_bias, 0, 255);

					const uint32_t packed = PACK_RGBA(
						static_cast<uint32_t>(r),
						static_cast<uint32_t>(g),
						static_cast<uint32_t>(b),
						static_cast<uint32_t>(a));
					reinterpret_cast<uint32_t*>(m_data)[y * m_width + x] = packed;
				}
		}
	}
	else
	{
		BB_ASSERT(false, "todo");
	}
}

void BBImage::SharpenImage(Allocator a_temp_allocator, const float a_intensity, const uint32_t a_thread_count)
{
	constexpr float sharpen3x3_factor = 1.f;
	constexpr float sharpen3x3_bias = 0;
	constexpr float sharpen3x3_filter[3 * 3]
	{
		-1, -1, -1,
		-1,  9, -1,
		-1, -1, -1
	};
	FilterImage(a_temp_allocator, sharpen3x3_filter, 3, 3, sharpen3x3_factor, sharpen3x3_bias, a_thread_count);
}

void BBImage::WriteAsBMP(const char* a_file_path)
{
	OSFileHandle write_bmp = CreateOSFile(a_file_path);

	BMP::File file = {};
	file.file_type = BMP::BMP_IMAGE_FILE_TYPE;
	file.offset_data = sizeof(BMP::File) + sizeof(BMP::Header) + sizeof(BMP::ColorHeader);
	file.file_size = file.offset_data + m_data_size;

	BMP::Header file_header = {};
	file_header.size = sizeof(BMP::Header) + sizeof(BMP::ColorHeader);
	file_header.width = m_width;
	file_header.height = m_height;
	file_header.bit_count = m_bit_count;

	BMP::ColorHeader color_header = {};
	color_header.red_mask = m_red_mask;
	color_header.green_mask = m_green_mask;
	color_header.blue_mask = m_blue_mask;
	color_header.alpha_mask = m_alpha_mask;
	color_header.color_space_type = BMP::BMP_RGBA_COLOR_SPACE;

	WriteToOSFile(write_bmp, &file, sizeof(BMP::File));
	WriteToOSFile(write_bmp, &file_header, sizeof(BMP::Header));
	WriteToOSFile(write_bmp, &color_header, sizeof(BMP::ColorHeader));
	WriteToOSFile(write_bmp, m_data, m_data_size);

	CloseOSFile(write_bmp);
}

void BBImage::LoadBMP(Allocator a_allocator, const char* a_file_path)
{
	const Buffer load_image = ReadOSFile(a_allocator, a_file_path);
	void* current_data = load_image.data;

	BMP::File file;
	BMP::Header file_header;
	BMP::ColorHeader color_header;

	Memory::Copy(&file, current_data, 1);
	current_data = Pointer::Add(current_data, sizeof(BMP::File));

	BB_ASSERT(file.file_type == 0x4D42, "tried to load a file that is not a BMP image!");

	Memory::Copy(&file_header, current_data, 1);
	current_data = Pointer::Add(current_data, sizeof(BMP::Header));

	if (file_header.size >= (sizeof(BMP::Header) + sizeof(BMP::ColorHeader)))
	{
		Memory::Copy(&color_header, current_data, 1);
		BB_ASSERT(BMP::isRGBA(color_header), "not supporting non-RGBA bitmaps yet");

		m_red_mask = 0x00ff0000;
		m_green_mask = 0x0000ff00;
		m_blue_mask = 0x000000ff;
		m_alpha_mask = 0xff000000;
	}

	current_data = Pointer::Add(load_image.data, file.offset_data);

	////some editors add extra info on the image file. Adjust for that.
	//if (file_header.bit_count == 32)
	//{
	//	file_header.size = sizeof(BMP::Header) + sizeof(BMP::ColorHeader);
	//	file.offset_data = sizeof(BMP::File) + sizeof(BMP::Header) + sizeof(BMP::ColorHeader);
	//}
	//else
	//{
	//	file_header.size = sizeof(BMP::Header);
	//	file.offset_data = sizeof(BMP::File) + sizeof(BMP::Header);
	//}
	//file.file_size = file.offset_data;

	BB_ASSERT(file_header.height > 0, "currently not supporting negative height BMP's.");

	m_width = file_header.width;
	m_height = file_header.height;
	m_bit_count = file_header.bit_count;
	m_data_size = static_cast<size_t>(file_header.width) * static_cast<size_t>(file_header.height) * file_header.bit_count / 8;
	m_data = BBalloc(a_allocator, m_data_size);

	//check to account for padding
	if (file_header.width % 4 == 0)
	{
		memcpy(m_data, current_data, m_data_size);
		file.file_size += m_data_size;
	}
	else
	{
		m_row_stride = file_header.width * file_header.bit_count / 8;
		const uint32_t new_stride = Pointer::AlignForwardAdjustment(m_row_stride, 4);
		void* cur_pos = m_data;
		for (int32_t i = 0; i < file_header.height; i++) //copy stride by stride.
		{
			memcpy(cur_pos, current_data, m_row_stride);
			memset(Pointer::Add(current_data, m_row_stride), 0, new_stride);
			current_data = Pointer::Add(current_data, m_row_stride);
			cur_pos = Pointer::Add(cur_pos, new_stride);
		}
		file.file_size += m_data_size + static_cast<size_t>(file_header.height) * new_stride;
	}
}