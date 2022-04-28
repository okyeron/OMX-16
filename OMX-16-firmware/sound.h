#pragma once

#include <FixedPoints.h>

using sample_t = SFixed<2, 13>;

constexpr sample_t SAMPLE_ZERO = sample_t(0);
constexpr sample_t SAMPLE_UNIT = sample_t(1);
constexpr sample_t SAMPLE_POS_ONE = sample_t(1.0);
constexpr sample_t SAMPLE_NEG_ONE = sample_t(-1.0);

constexpr float SAMPLE_RATE_TARGET = 48000.0;
constexpr long SAMPLE_RATE_CPU_DIVISOR = F_CPU / (long)SAMPLE_RATE_TARGET;
constexpr float SAMPLE_RATE = (float)F_CPU / (float)SAMPLE_RATE_CPU_DIVISOR;


class SoundSource {
public:
  virtual void supply(sample_t* buffer, int count) = 0;
    // NB: Will get called at interrupt time!
};


class TriangleToneSource : public SoundSource {
public:
  TriangleToneSource();

  void playNote(float freq, float dur);

  virtual void supply(sample_t* buffer, int count);

private:
  UFixed<0, 32> theta;  // current location in cycle
  UFixed<0, 32> delta;  // amount of theta per sample

  UFixed<0, 32> amp;
  UFixed<0, 32> decay;
};

class Samples {
public:
  Samples() : samples(nullptr), sampleCount(0) { }
  Samples(void* data, size_t len)
    : samples((sample_t*)data), sampleCount(len / sizeof(sample_t)) { }

  using sample_t = SFixed<0, 7>;

  sample_t operator[](int n) const { return samples[n]; }
  int      length()          const { return sampleCount; }

private:
  sample_t* samples;
  int sampleCount;

};

class SampleSourceBase : public SoundSource {
public:
  SampleSourceBase();
  void load(const Samples& s);
  void play(float amp);

protected:
  Samples samples;
  int nextSample;

  using comp_t = SFixed<15, 16>;
  comp_t amp;
};

template<int sample_rate>
class SampleSource : public SampleSourceBase {
public:
  SampleSource() { }
  virtual void supply(sample_t* buffer, int count);
};



class SampleGateSourceBase : public SoundSource {
public:
  SampleGateSourceBase();
  void load(const Samples& s);

  void gate(float a);
  void gateOff();

  void setPosition(float);

protected:
  virtual int sampleRate() const = 0;

  Samples samples;
  bool looped;
  int startSample;
  int nextSample;

  using amp_t = UFixed<0, 32>;

  amp_t amp;
  amp_t ampTarget;
};

template<int sample_rate>
class SampleGateSource : public SampleGateSourceBase {
public:
  SampleGateSource() { }
  virtual void supply(sample_t* buffer, int count);
protected:
  virtual int sampleRate() const { return sample_rate; }
};


class MixSource : public SoundSource {
public:
  MixSource(SoundSource& s1, SoundSource& s2);

  virtual void supply(sample_t* buffer, int count);

private:
  SoundSource& s1;
  SoundSource& s2;
};


class FilterSource : public SoundSource {
public:
  FilterSource(SoundSource& in);
  void setFreqAndQ(float freq, float q);

  virtual void supply(sample_t* buffer, int count);

private:
  SoundSource& in;

  sample_t f;
  sample_t fb;

  sample_t b0;
  sample_t b1;
};


class DelaySource : public SoundSource {
public:
  DelaySource(SoundSource& in);
  void setDelayMod(float);    // 1.0 is base delay length
  void setFeedback(float);    // in range 0.0 to 1.0 (careful!)


  virtual void supply(sample_t* buffer, int count);

  static constexpr float maxDelay = 0.150;
  static constexpr float baseDelay = 0.080;
  static constexpr float minDelay = 0.001;
  static constexpr float maxMod = maxDelay / baseDelay;
  static constexpr float minMod = minDelay / baseDelay;

private:
  SoundSource& in;

  using delay_t = SFixed<15,16>;
  delay_t delay;
  delay_t delayTarget;

  sample_t feedback;
  sample_t feedbackTarget;

  static constexpr int maxDelaySamples = maxDelay * SAMPLE_RATE;
  static constexpr int baseDelaySamples = baseDelay * SAMPLE_RATE;

  static_assert(maxDelaySamples < 1 << delay_t::IntegerSize,
    "delay_t integer portion isn't big enough");

  sample_t tank[maxDelaySamples];
  int writeP;
};
