#include "C:\\CPP\\wav_writer\\wav_writer.h"
#define _USE_MATH_DEFINES
#include <math.h>

void assign_header_info(WaveFile *p_wavefile, int num_channels, int num_samples, double dt)
{
	p_wavefile->num_frames = num_samples;

	strcpy(p_wavefile->riff_marker, "RIFF");
//	printf("riff marker is %s\n", p_wavefile->riff_marker);

	/* Need to calculate file size */

	strcpy(p_wavefile->file_type_header, "WAVE");
	strcpy(p_wavefile->format_chunk_marker, "fmt ");
	p_wavefile->format_data_length = 16; 
	p_wavefile->format_type = 1; // PCM
	p_wavefile->num_channels = num_channels;
	p_wavefile->sample_rate = 1 / dt;
	p_wavefile->bits_per_sample = 16; // This should probably be passed in
	p_wavefile->bitrate_math = p_wavefile->num_channels * p_wavefile->bits_per_sample / 8;
	p_wavefile->byterate = num_samples * p_wavefile->bitrate_math;
	p_wavefile->dt = dt;

	strcpy(p_wavefile->data_chunk_header, "data");

	/* Need to calculate data section size */
	p_wavefile->data_section_size = p_wavefile->num_frames * p_wavefile->bitrate_math;
	p_wavefile->file_size = p_wavefile->data_section_size + 16 + 28;
}

void generate_stereo_sine(WaveFile *p_wavefile, short A, double f, double dt, double ph, int num_samples)
{
	int i, j;
	int num_channels = 2;

	p_wavefile->channel_samples = (short **)calloc(num_channels, sizeof(*p_wavefile->channel_samples));

	for (i = 0; i < num_channels; i++)
	{
		p_wavefile->channel_samples[i] = (short *)calloc(num_samples, sizeof(**p_wavefile->channel_samples));
		for (j = 0; j < num_samples; j++)
		{
			p_wavefile->channel_samples[i][j] = A * sin(2.0 * M_PI * f * j * dt + ph);
//			printf("Channel sample generated: %d\n", p_wavefile->channel_samples[i][j]);
		}
	}
	
	assign_header_info(p_wavefile, num_channels, num_samples, dt);
}

void generate_stereo_sine_wrapper(WaveFile *p_wavefile, short A, double f, double duration, double sample_freq)
{
	double dt = 1.0 / sample_freq;
	int num_samples = duration / dt;
	generate_stereo_sine(p_wavefile, A, f, dt, 0, num_samples);
}

void generate_double_sine(WaveFile *p_wavefile, short A1, short A2, double f1, double f2, double dt,
		double ph, int num_samples, int num_channels)
{
	int i, j;
//	int num_channels = 2;

	p_wavefile->channel_samples = (short **)calloc(num_channels, sizeof(*p_wavefile->channel_samples));

	for (i = 0; i < num_channels; i++)
	{
		p_wavefile->channel_samples[i] = (short *)calloc(num_samples, sizeof(**p_wavefile->channel_samples));
		for (j = 0; j < num_samples; j++)
		{
			p_wavefile->channel_samples[i][j] = A1 * sin(2.0 * M_PI * f1 * j * dt + ph) +
				A2 * sin(2.0 * M_PI * f2 * j * dt + ph);
		}
	}

	assign_header_info(p_wavefile, num_channels, num_samples, dt);
}

void generate_double_sine_wrapper(WaveFile *p_wavefile, short A1, short A2, double f1, double f2, 
		double duration, double sample_freq, int num_channels)
{
	double dt = 1.0 / sample_freq;
	int num_samples = duration / dt;
	generate_double_sine(p_wavefile, A1, A2, f1, f2, dt, 0, num_samples, num_channels);
}

int main()
{
	char output_filename[128] = "output_test.wav";
	WaveFile wavefile = { 0 };

	printf("Generating a stereo sine wave\n");
//	generate_stereo_sine(&wavefile, 2000, 440, 0.0045, 0, 880);
//	generate_stereo_sine(&wavefile, 2000, 440, 0.00114, 0, 1760);
//	generate_stereo_sine(&wavefile, 2000, 440, 2.2727e-4, 0, 1760);
//	generate_stereo_sine_wrapper(&wavefile, 2000, 880, 5, 8800);
//	generate_double_sine_wrapper(&wavefile, 1000, 1000, 440, 880, 5, 8800, 2);
	generate_double_sine_wrapper(&wavefile, 1000, 1000, 440, 880, 2, 8800, 1);

	print_header(wavefile);
	write_wave(wavefile, output_filename);

	destroy_wavearrays(&wavefile);
	return 0;
}
