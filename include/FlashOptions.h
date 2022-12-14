#pragma once
#define GLOBAL_OPTIONS_SECTOR 0xec

typedef struct Global_options
{
	int show_fps;
	int show_heap;
	int buffering;
	int spi_queue;

	int CPU_speed;
	int SPI_speed;
	uint8_t GammaPOS[19];
	uint8_t GammaNEG[19];
} Global_options;

void init_global_options();

void set_def_gamma();
void set_def_settings();
void load_settings();
void save_settings();
void draw_settings();
void apply_system_settings();

extern Global_options Gl_options;