#include "C:\\CPP\\wav_writer\\wav_writer.h"
#include "wav_generator.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdarg.h>

/*#define A4 440
#define C5 523.2511
#define E5 659.2551
#define A5 880
*/
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
//	p_wavefile->channel_samples = (short **)calloc(num_channels, sizeof(short *));

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

	printf("Generating double sine\n");

	p_wavefile->channel_samples = (short **)calloc(num_channels, sizeof(*p_wavefile->channel_samples));

	for (i = 0; i < num_channels; i++)
	{
//		printf("Generating %d samples for channel %d\n", num_samples, i);
		p_wavefile->channel_samples[i] = (short *)calloc(num_samples, sizeof(**p_wavefile->channel_samples));
//		p_wavefile->channel_samples[i] = (short *)calloc(num_samples, sizeof(short));
//		printf("Allocated memory for channel %d\n", i);
		for (j = 0; j < num_samples; j++)
		{
//			printf("Generating sample %d\n", j);
			p_wavefile->channel_samples[i][j] = A1 * sin(2.0 * M_PI * f1 * j * dt + ph) +
				A2 * sin(2.0 * M_PI * f2 * j * dt + ph);
		}
	}

	printf("Assigning header info\n");
	assign_header_info(p_wavefile, num_channels, num_samples, dt);
}

void generate_double_sine_wrapper(WaveFile *p_wavefile, short A1, short A2, double f1, double f2, 
		double duration, double sample_freq, int num_channels)
{
	double dt = 1.0 / sample_freq;
	int num_samples = duration / dt;
	generate_double_sine(p_wavefile, A1, A2, f1, f2, dt, 0, num_samples, num_channels);
}

void generate_generic_sine(WaveFile *p_wavefile, short Ai, double dt, double ph, int num_samples, int num_channels, double frequency, int freq_count)
{
//	va_list args;
//	va_start(args, num_frequencies);
//	double frequency;
	int i,j,k;

//	short Ai = A_net / num_frequencies; // Should this be weighted differently?

	if (0 == freq_count)
		p_wavefile->channel_samples = (short **)calloc(num_channels, sizeof(*p_wavefile->channel_samples));

	printf("Frequency: %f\n", frequency);
#if 1
	for (i = 0; i < num_channels; i++)
	{
		printf("Channel %d\n", i);
		if (0 == freq_count)
			p_wavefile->channel_samples[i] = (short *)calloc(num_samples, sizeof(**p_wavefile->channel_samples));
		for (j = 0; j < num_samples; j++)
		{
//			printf("Sample %d\n", j);
//			for (k = 0; k < num_samples; k++)
//			{
//				frequency = va_arg(args, double);
			p_wavefile->channel_samples[i][j] += Ai * sin(2.0 * M_PI * frequency * j * dt + ph);
//			}
		}
	}
#endif
}

void generate_generic_sine_wrapper(WaveFile *p_wavefile, short A_net, double duration, double sample_freq, 
		int num_channels, int num_frequencies, ...)
{
	va_list args;
	double dt = 1.0 / sample_freq;
	int num_samples = duration / dt;
	
	short Ai = A_net / num_frequencies; // Should this be weighted differently?

	double frequency;
	double ph = 0;

	int i;
	va_start(args, num_frequencies);
	for (i = 0; i < num_frequencies; i++)
	{
		frequency = va_arg(args, double);
//		printf("outer frequency: %f\n", frequency);
		generate_generic_sine(p_wavefile, Ai, dt, ph, num_samples, num_channels, frequency, i);
	}
	va_end(args);

	printf("Assigning header info\n");
	assign_header_info(p_wavefile, num_channels, num_samples, dt);
}

typedef struct FreqAmp
{
	double frequency;
	double amplitude;
} FreqAmp;

void generate_generic_sine_wrapper_multi_amp(WaveFile *p_wavefile, double duration, double sample_freq, 
		int num_channels, int num_frequencies, ...)
{
	va_list args;
	double dt = 1.0 / sample_freq;
	int num_samples = duration / dt;

	double ph = 0;
	FreqAmp freq_weight = { 0 };

	int i;
	va_start(args, num_frequencies);
	for (i = 0; i < num_frequencies; i++)
	{
		freq_weight = va_arg(args, FreqAmp);
		generate_generic_sine(p_wavefile, freq_weight.amplitude, dt, ph, num_samples, num_channels,
				freq_weight.frequency, i);
	}
	va_end(args);

	printf("Assigning header info\n");
	assign_header_info(p_wavefile, num_channels, num_samples, dt);
}


int main()
{
	char output_filename[128] = "output_test.wav";
	WaveFile wavefile = { 0 };

	FreqAmp freq_weight[8] = {
		{419.04,840.8858},
		{838.24, 1533.618}, 
		{1257.12, 167.1285},
	       	{1676.64, 122.6765}, 
		{2095.68, 88.09782},
		{2514.88, 155.0136}, 
		{2933.76, 62.45731}, 
		{3353.18, 30.12254}
	};

	printf("Generating a stereo sine wave\n");
//	generate_stereo_sine(&wavefile, 2000, 440, 0.0045, 0, 880);
//	generate_stereo_sine(&wavefile, 2000, 440, 0.00114, 0, 1760);
//	generate_stereo_sine(&wavefile, 2000, 440, 2.2727e-4, 0, 1760);
//	generate_stereo_sine_wrapper(&wavefile, 2000, 880, 5, 8800);
//	generate_double_sine_wrapper(&wavefile, 1000, 1000, 440, 880, 5, 8800, 2);
//	generate_double_sine_wrapper(&wavefile, 1000, 1000, 440, 880, 2, 8800, 1);
//	generate_double_sine_wrapper(&wavefile, 1000, 1000, A4, A5, 2, A5*10, 1);
//	generate_double_sine_wrapper(&wavefile, 1000, 1000, A4, C5, 2, C5*10, 1);
//	generate_double_sine_wrapper(&wavefile, 1000, 1000, C5, Ef5, 2, Ef5*10, 1);

//	generate_generic_sine_wrapper(&wavefile, 3000, 2, 880, 2, 4, 220.0, 440.0, 880.0, 1000.0);
//	generate_generic_sine_wrapper(&wavefile, 4000, 2, 880.0 * 2.0, 2, 2, 440.0, 880.0);
//	generate_generic_sine_wrapper(&wavefile, 3000, 2, 880.0 * 10, 1, 2, 440.0, 880.0);
//	generate_generic_sine_wrapper(&wavefile, 3000, 2, G5 * 10, 1, 3, C5, Ef5, G5);

	generate_generic_sine_wrapper_multi_amp(&wavefile, 2, 3353.28*10.0, 1, 8, freq_weight[0], 
			freq_weight[1], freq_weight[2], freq_weight[3], freq_weight[4], freq_weight[5],
			freq_weight[6], freq_weight[7]);

	print_header(wavefile);
	write_wave(wavefile, output_filename);

	destroy_wavearrays(&wavefile);
	return 0;
}
