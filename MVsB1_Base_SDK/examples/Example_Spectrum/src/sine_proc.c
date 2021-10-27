#include "type.h"
#include <math.h>

static float pointers[441*4];
#define PI (3.14159265358979f)
//#define PI acos(-1)

const uint16_t convert_table[2][9] =
{
  {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000},
  {8, 441*4, 12, 16, 441*2, 24, 32, 441, 48}
};

/**
 * [sine_generator description]
 * @param  samplerate for the sample rate of the sine, support 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
 * @param  frq        frequency of the signal
 * @param  ampl       amplitude of the signal, should be a negative value like -8, -60
 * @param  is_24bit   is 24 bit or 16 bit, TRUE for 24 bit PCM,, FALSE for 16 bit PCM
 * @param  dataout    data out put buffer
 * @param  pLen       data out put length in bytes
 * @return            TRUE for success, FALSE for failed.
 */
bool sine_generator(uint16_t samplerate, uint16_t frq, int8_t ampl, bool is_24bit, int32_t *dataout, uint16_t *pLen)
{
  int i;
  uint8_t index = 0;
  uint16_t sample_len;
  int16_t *outp16 = (int16_t*)dataout;
  int32_t *outp32 = (int32_t*)dataout;;
  for(i=0; i<9; i++)
  {
    if(convert_table[0][i] == samplerate)
    {
      index = i;
      break;
    }
  }

  if(index > 8)
    return FALSE;

  sample_len = convert_table[1][index];
  //printf("index=%d, %d\n", index, sample_len);

  if(is_24bit)
  {
    for(i=0; i<sample_len; i++)
    {
      pointers[i] = sinf(2 * PI * frq * i / samplerate);
      if((int32_t)roundf(pow(10.0f, ampl/20.0f) * pointers[i] * 0x800000) == 0x800000)
      {
        outp32[i * 2] = (int32_t)0x800000 - 1;
        outp32[i * 2 + 1] = (int32_t)0x800000 - 1;
      }
      else
      {
        outp32[i * 2] = (int32_t)roundf(pow(10.0f, ampl/20.0f) * pointers[i] * 0x800000);
        outp32[i * 2 + 1] = (int32_t)roundf(pow(10.0f, ampl/20.0f) * pointers[i] * 0x800000);
      }
      //printf("%d: %f, %d, %d\n", i, pointers[i], outp32[i * 2], outp32[i * 2 + 1]);
    }
  }
  else
  {
    for(i=0; i<sample_len; i++)
    {
      pointers[i] = sinf(2 * PI * frq * i / samplerate);
      if((int32_t)roundf(pow(10.0f, ampl/20.0f) * pointers[i] * 0x8000) == 0x8000)
      {
      	outp16[i * 2] = (int16_t)0x8000 - 1;
      	outp16[i * 2 + 1] = (int16_t)0x8000 - 1;
      }
      else
      {
      	outp16[i * 2] = (int16_t)roundf(pow(10.0f, ampl/20.0f) * pointers[i] * 0x8000);
      	outp16[i * 2 + 1] = (int16_t)roundf(pow(10.0f, ampl/20.0f) * pointers[i] * 0x8000);
      }
      //printf("%f, %d, %d\n", pointers[i], outp16[i * 2], outp16[i * 2 + 1]);
    }
  }

  *pLen = sample_len * 4 * (is_24bit?2:1);
  //printf("DONE: %d\n", *pLen);
  //return sin(30.0f * PI/180);
  return TRUE;
}
