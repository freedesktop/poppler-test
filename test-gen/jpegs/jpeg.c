#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <stdint.h>
#include <stdbool.h>

void argb32_to_rgb24(void *in, unsigned char *out, int n)
{
	uint32_t *in_d = in;
	while (n--) {
		uint32_t d = *in_d;
		out[0] = d >> 16 & 0xff;
		out[1] = d >> 8 & 0xff;
		out[2] = d >> 0 & 0xff;
		in_d++;
		out+=3;
	}
}

void write_jpeg(char *name, J_COLOR_SPACE in_color_space, J_COLOR_SPACE color_space, int components, boolean write_Adobe_marker, boolean write_JFIF_header)
{
	cairo_surface_t *s = cairo_image_surface_create_from_png("romedalen.png");
	unsigned char *data = cairo_image_surface_get_data(s);
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	FILE *outfile = fopen(name, "wb");
	jpeg_stdio_dest(&cinfo, outfile);
	cinfo.image_width = cairo_image_surface_get_width(s);
	cinfo.image_height = cairo_image_surface_get_height(s);
	cinfo.input_components = components;
	cinfo.in_color_space = in_color_space;

	jpeg_set_defaults(&cinfo);
	jpeg_set_colorspace(&cinfo, color_space);
	cinfo.write_Adobe_marker = write_Adobe_marker;
	cinfo.write_JFIF_header = write_JFIF_header;
	jpeg_set_quality(&cinfo, 50, TRUE);
	jpeg_start_compress(&cinfo, TRUE);
	int width = cairo_image_surface_get_width(s);
	int ppb = components;
	int row_stride = width * 4;
	unsigned char *rgb24_buffer = malloc(width * ppb);
	while (cinfo.next_scanline < cinfo.image_height) {
		unsigned char *row_pointer[1];
		if (components == 3) {
			argb32_to_rgb24(&data[cinfo.next_scanline * row_stride], rgb24_buffer, width);
			row_pointer[0] = rgb24_buffer;
		} else {
			row_pointer[0] = &data[cinfo.next_scanline * row_stride];
		}
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	fclose(outfile);
	jpeg_destroy_compress(&cinfo);
}

int main()
{
	write_jpeg("out-rgb-rgb-adobe.jpg", JCS_RGB, JCS_RGB, 3, true, false);
	write_jpeg("out-rgb-rgb.jpg", JCS_RGB, JCS_RGB, 3, false, false);
	write_jpeg("out-rgb-yuv-jfif.jpg", JCS_RGB, JCS_YCbCr, 3, false, true);
	write_jpeg("out-rgb-yuv.jpg", JCS_RGB, JCS_YCbCr, 3, false, false);
	/* unsupported color space transformation */
	//write_jpeg("out-yuv-rgb-adobe.jpg", JCS_YCbCr, JCS_RGB, 3, true, false);
	//write_jpeg("out-yuv-rgb.jpg", JCS_YCbCr, JCS_RGB, 3, false, false);
	write_jpeg("out-yuv-yuv-jfif.jpg", JCS_YCbCr, JCS_YCbCr, 3, false, true);
	write_jpeg("out-yuv-yuv.jpg", JCS_YCbCr, JCS_YCbCr, 3, false, false);
	write_jpeg("out-cmyk-cmyk-adobe.jpg", JCS_CMYK, JCS_CMYK, 4, true, false);
	write_jpeg("out-cmyk-cmyk.jpg", JCS_CMYK, JCS_CMYK, 4, false, false);
	write_jpeg("out-ycck-ycck-adobe.jpg", JCS_YCCK, JCS_YCCK, 4, true, false);
	write_jpeg("out-ycck-ycck.jpg", JCS_YCCK, JCS_YCCK, 4, false, false);
	return 0;
}
