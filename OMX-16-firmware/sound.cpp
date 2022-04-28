#include "sound.h"
#include "types.h"

namespace {
  inline void silence(sample_t*& buffer, int& count) {
    while(count--) *buffer++ = SAMPLE_ZERO;
  }
}

TriangleToneSource::TriangleToneSource()
  : theta(0), delta(0), amp(0), decay(0)
  { }

void TriangleToneSource::playNote(float freq, float dur) {
  delta = freq / SAMPLE_RATE;

  static const UFixed<0, 32> quarter = 0.25;
  theta = quarter;

  amp = 0.9;
  decay = 0.9f * dur / (float)SAMPLE_RATE;
}

void TriangleToneSource::supply(sample_t* buffer, int count) {
  using sample_fixed_t = SFixed<15, 16>;

  while (count--) {
    sample_fixed_t s(theta);

    static const sample_fixed_t half(0.5);
    if (s >= half) s = 1 - s;

    static const sample_fixed_t scale(2 * SAMPLE_UNIT);
    s = s * scale;

    sample_fixed_t a(amp);
    s = s * a;

    static const sample_fixed_t zero_offset(SAMPLE_ZERO);
    s += zero_offset;

    *buffer++ = (sample_t)(s.getInteger());

    theta += delta;
    amp = amp > decay ? amp - decay : 0;
  }
}


SampleSourceBase::SampleSourceBase()
  : nextSample(samples.length())
  { }

void SampleSourceBase::load(const Samples& s) {
  samples = s;
  nextSample = samples.length();
}

void SampleSourceBase::play(float ampf) {
  amp = ampf;
  nextSample = 0;
}

template<>
void SampleSource<int(SAMPLE_RATE)>::supply(sample_t* buffer, int count) {
  while (nextSample < samples.length() && count) {
    count -= 1;
    *buffer++ = sample_t(samples[nextSample++]*amp);
  }
  silence(buffer, count);
}

template<>
void SampleSource<int(SAMPLE_RATE/2)>::supply(sample_t* buffer, int count) {
  comp_t a = amp/2;

  while (nextSample < samples.length() && count) {
    count -= 2;
    comp_t v = comp_t(samples[nextSample++]);
    comp_t w = nextSample < samples.length() ? comp_t(samples[nextSample]) : v;

    *buffer++ = sample_t((v + v) * a);
    *buffer++ = sample_t((v + w) * a);
  }
  silence(buffer, count);
}

template<>
void SampleSource<int(SAMPLE_RATE/4)>::supply(sample_t* buffer, int count) {
  comp_t a = amp/4;

  while (nextSample < samples.length() && count) {
    count -= 4;
    comp_t v = comp_t(samples[nextSample++]);
    comp_t w = nextSample < samples.length() ? comp_t(samples[nextSample]) : v;

    *buffer++ = sample_t((v + v + v + v) * a);
    *buffer++ = sample_t((v + v + v + w) * a);
    *buffer++ = sample_t((v + v + w + w) * a);
    *buffer++ = sample_t((v + w + w + w) * a);
  }
  silence(buffer, count);
}

SampleGateSourceBase::SampleGateSourceBase()
  : looped(false), startSample(0), nextSample(0), amp(0), ampTarget(0)
  { }

void SampleGateSourceBase::load(const Samples& s) {
  samples = s;
  looped = samples.length() >= sampleRate() / 2;
  startSample = 0;
  nextSample = 0;
}
void SampleGateSourceBase::gate(float a) {
  if (ampTarget == amp_t(0))
    nextSample = startSample;

  ampTarget = amp_t(a);
}

void SampleGateSourceBase::gateOff() {
  ampTarget = amp_t(0);
}

void SampleGateSourceBase::setPosition(float p) {
  int l = samples.length();
  if (!looped) return;

  startSample = clamp(int(float(l)*p), 0, l - 1);
}

// TODO: Write the SAMPLE_RATE version of SampleGateSource::supply()

template<>
void SampleGateSource<int(SAMPLE_RATE/2)>::supply(sample_t* buffer, int count) {
  if (samples.length() > 0) {
    while (count) {
      if (!looped && nextSample >= samples.length()) break;
      SFixed<15, 16> v(samples[nextSample++]);
      if (looped && nextSample >= samples.length()) nextSample = 0;

      SFixed<15, 16> w(samples[nextSample]);
      SFixed<15, 16> a(amp);
      a /= 2;

      *buffer++ = sample_t((v + v) * a);
      *buffer++ = sample_t((v + w) * a);
      count -= 2;

      /*
        slew = (t * SR/2) root (-20dB)
      */
      constexpr amp_t slewUp(  0.92316169902);   // 1.2ms
      constexpr amp_t slewDown(0.99887191858);   // 85ms

      if (amp < ampTarget)  amp = ampTarget - (ampTarget - amp) * slewUp;
      else                  amp = ampTarget + (amp - ampTarget) * slewDown;
    }
  }
  silence(buffer, count);
}

template<>
void SampleGateSource<int(SAMPLE_RATE/4)>::supply(sample_t* buffer, int count) {
  if (samples.length() > 0) {
    while (count) {
      if (!looped && nextSample >= samples.length()) break;
      SFixed<15, 16> v(samples[nextSample++]);
      if (looped && nextSample >= samples.length()) nextSample = 0;

      SFixed<15, 16> w(samples[nextSample]);
      SFixed<15, 16> a(amp);
      a /= 4;

      *buffer++ = sample_t((v + v + v + v) * a);
      *buffer++ = sample_t((v + v + v + w) * a);
      *buffer++ = sample_t((v + v + w + w) * a);
      *buffer++ = sample_t((v + w + w + w) * a);
      count -= 4;

      /*
        slew = (t * SR/4) root (-20dB)
      */
      constexpr amp_t slewUp(  0.85222752254f);   // 1.2ms
      constexpr amp_t slewDown(0.99774510973f);   // 85ms

      if (amp < ampTarget)  amp = ampTarget - (ampTarget - amp) * slewUp;
      else                  amp = ampTarget + (amp - ampTarget) * slewDown;
    }
  }
  silence(buffer, count);
}


MixSource::MixSource(SoundSource& _s1, SoundSource& _s2)
  : s1(_s1), s2(_s2)
  { }

void MixSource::supply(sample_t* buffer, int count) {
  s1.supply(buffer, count);

  sample_t* buf2 = (sample_t*)(alloca(sizeof(sample_t) * count));
  s2.supply(buf2, count);

  for (int i = 0; i < count; ++i) {
    sample_t s = (*buffer + *buf2++);
    *buffer++ = s;
  }
}

FilterSource::FilterSource(SoundSource& _in)
  : in(_in), b0(0), b1(0)
{
  setFreqAndQ(2540.0f, 0.2f);
}

void FilterSource::setFreqAndQ(float freq, float q)
{
  constexpr float freq_max = SAMPLE_RATE / 6.0f;
  freq = min(freq, freq_max);
  q = clamp(q, 0.0f, 0.9f);

  constexpr float c = PI / SAMPLE_RATE;
  constexpr bool accurate = true;

  float ff;

  if (accurate)
    ff = 2.0f * sinf(c * freq);
  else {
    constexpr float d = 2.0f * c;
    ff = d * freq;
    if (ff > 2.0f) ff = 2.0f;
  }

  // q needs to roll off as it approaches freq_max or "bad things"(tm) happen!
  constexpr float fullQfreq = 1000.0f;
  q *= (1.0f - (q - fullQfreq)/(freq_max - fullQfreq));

  f = ff;
  fb = q + q/(1.0f - ff);
}

void FilterSource::supply(sample_t* buffer, int count) {
  in.supply(buffer, count);

  while (count--) {
    sample_t s_in = *buffer;

    b0 = b0 + f * (s_in - b0 + fb * (b0 - b1));
    b1 = b1 + f * (b0 - b1);

    *buffer++ = b1;
  }
}

DelaySource::DelaySource(SoundSource& _in)
  : in(_in),
  delay(baseDelaySamples), delayTarget(delay),
  feedback(0.35f), feedbackTarget(feedback),
  writeP(0)
 {
   for (int i = 0; i < maxDelaySamples; i++) tank[i] = sample_t(0);
 }

void DelaySource::setFeedback(float f) {
  constexpr sample_t f_min(0.0f);
  constexpr sample_t f_max(0.995f);

  feedbackTarget = clamp(sample_t(f), f_min, f_max);
}

void DelaySource::setDelayMod(float d) {
  constexpr delay_t d_min(1);
  constexpr delay_t d_max(maxDelaySamples);
  constexpr delay_t d_base(baseDelaySamples);

  delayTarget = clamp(delay_t(d) * d_base, d_min, d_max);
}

void DelaySource::supply(sample_t* buffer, int count) {
  in.supply(buffer, count);
  while (count--) {
    int readP = writeP + int(delay);
    sample_t v = tank[readP % maxDelaySamples];
    sample_t in = *buffer;

    constexpr sample_t two(2.0);
    constexpr sample_t negtwo(-2.0);
    sample_t w = in + feedback * v;
    w = clamp(w, negtwo, two);

    tank[writeP] = w;
    writeP = (writeP > 0 ? writeP : maxDelaySamples) - 1;
    *buffer++ = w;

    constexpr delay_t  d_exp(0.99988487737);  // -24dB over 500ms @ 48kHz
    constexpr sample_t f_exp(0.97162795158);  // -24dB over 2ms @ 48kHz

    delay = delayTarget - (delayTarget - delay) * d_exp;
    feedback = feedbackTarget - (feedbackTarget - feedback) * f_exp;
  }
}