/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is MPEG4IP.
 * 
 * The Initial Developer of the Original Code is Cisco Systems Inc.
 * Portions created by Cisco Systems Inc. are
 * Copyright (C) Cisco Systems Inc. 2000, 2001.  All Rights Reserved.
 * 
 * Contributor(s): 
 *              Bill May        wmay@cisco.com
 *
 * Adapted to PD/PDP by Yves Degoyon (ydegoyon@free.fr)
 */

/*
 * audio.cpp provides an interface (CPDPAudioSync) between the codec and
 * the SDL audio APIs.
 */
#include <stdlib.h>
#include <string.h>
#include "pdp_mp4playersession.h"
#include "pdp_mp4audiosync.h"
#include "player_util.h"
#include "our_config_file.h"
#include "m_pd.h"

#define audio_message(loglevel, fmt...) message(loglevel, "audiosync", fmt)

static void pdp_audio_callback (void *userdata, Uint8 *stream, int len)
{
  CPDPAudioSync *a = (CPDPAudioSync *)userdata;
  a->audio_callback(stream, len);
}

CPDPAudioSync::CPDPAudioSync (CPlayerSession *psptr, t_pdp_mp4player *pdp_father) : CAudioSync(psptr)
{
  m_fill_index = m_play_index = 0;
  for (int ix = 0; ix < DECODE_BUFFERS_MAX; ix++) {
    m_buffer_filled[ix] = 0;
    m_sample_buffer[ix] = NULL;
  }
  m_buffer_size = 0;
  m_config_set = 0;
  m_audio_initialized = 0;
  m_audio_paused = 1;
  m_resync_required = 0;
  m_dont_fill = 0;
  m_consec_no_buffers = 0;
  m_audio_waiting_buffer = 0;
  m_skipped_buffers = 0;
  m_didnt_fill_buffers = 0;
  m_play_time = 0         ;
  m_buffer_latency = 0;
  m_first_time = 1;
  m_first_filled = 1;
  m_buffer_offset_on = 0;
  m_buffer_ts = 0;
  m_load_audio_do_next_resync = 0;
  m_convert_buffer = NULL;
  m_father = pdp_father;
}

CPDPAudioSync::~CPDPAudioSync (void)
{
  for (int ix = 0; ix < DECODE_BUFFERS_MAX; ix++) {
    if (m_sample_buffer[ix] != NULL)
      free(m_sample_buffer[ix]);
    m_sample_buffer[ix] = NULL;
  }
  CHECK_AND_FREE(m_convert_buffer);
  audio_message(LOG_NOTICE, 
		"Audio sync skipped %u buffers", 
		m_skipped_buffers);
  audio_message(LOG_NOTICE, "didn't fill %u buffers", m_didnt_fill_buffers);
}

void CPDPAudioSync::set_config (int freq, 
			     int channels, 
			     int format, 
			     uint32_t sample_size) 
{
  if (m_config_set != 0) 
    return;
  
  if (format == AUDIO_U8 || format == AUDIO_S8)
    m_bytes_per_sample = 1;
  else
    m_bytes_per_sample = 2;

  if (sample_size == 0) {
    int temp;
    temp = freq;
    while ((temp & 0x1) == 0) temp >>= 1;
    sample_size = temp;
    while (sample_size < 1024) sample_size *= 2;
    while (((sample_size * 1000) % freq) != 0) sample_size *= 2;
  } 
  
  m_buffer_size = channels * sample_size * m_bytes_per_sample;

  for (int ix = 0; ix < DECODE_BUFFERS_MAX; ix++) {
    m_buffer_filled[ix] = 0;
    m_sample_buffer[ix] = (uint8_t *)malloc(2 * m_buffer_size);
  }
  m_freq = freq;
  m_channels = channels;
  m_format = format;
  if (m_format == AUDIO_U8) {
    m_silence = 0x80;
  } else {
    m_silence = 0x00;
  }
  m_config_set = 1;
  m_msec_per_frame = (sample_size * 1000) / m_freq;
  audio_message(LOG_DEBUG, "buffer size %d msec per frame %d", m_buffer_size, m_msec_per_frame);
};

uint8_t *CPDPAudioSync::get_audio_buffer (void)
{
  int ret;
  int locked = 0;
  if (m_dont_fill == 1) {
    return (NULL);
  }

  if (m_audio_initialized != 0) {
    locked = 1;
  }
  ret = m_buffer_filled[m_fill_index];
  if (ret == 1) {
    m_audio_waiting_buffer = 1;
    m_audio_waiting_buffer = 0;
    if (m_dont_fill != 0) {
      return (NULL);
    }
    locked = 0;
    if (m_audio_initialized != 0) {
      locked = 1;
    }
    ret = m_buffer_filled[m_fill_index];
    if (locked)
    if (ret == 1) {
      post("pdp_mp4audiosync : no buffer");
      return (NULL);
    }
  }
  return (m_sample_buffer[m_fill_index]);
}

void CPDPAudioSync::load_audio_buffer (uint8_t *from, 
				       uint32_t bytes, 
				       uint64_t ts, 
				       int resync)
{
  uint8_t *to;
  uint32_t copied;
  copied = 0;
  if (m_buffer_offset_on == 0) {
    int64_t diff = ts - m_buffer_ts;

    if (m_buffer_ts != 0 && diff > 1) {
      m_load_audio_do_next_resync = 1;
      audio_message(LOG_DEBUG, "timeslot doesn't match - %llu %llu",
		    ts, m_buffer_ts);
    }
    m_buffer_ts = ts;
  } else {
    int64_t check;
    check = ts - m_loaded_next_ts;
    if (check > m_msec_per_frame) {
      audio_message(LOG_DEBUG, "potential resync at ts "U64" should be ts "U64,
		    ts, m_loaded_next_ts);
      uint32_t left;
      left = m_buffer_size - m_buffer_offset_on;
      to = get_audio_buffer();
      memset(to + m_buffer_offset_on, 0, left);
      filled_audio_buffer(m_buffer_ts, 0);
      m_buffer_offset_on = 0;
      m_load_audio_do_next_resync = 1;
      m_buffer_ts = ts;
    }
  }
  m_loaded_next_ts = bytes * M_64;
  m_loaded_next_ts /= m_bytes_per_sample;
  m_loaded_next_ts /= m_freq;
  m_loaded_next_ts += ts;

  while ( bytes > 0) {
    to = get_audio_buffer();
    if (to == NULL) {
      return;
    }
    int copy;
    uint32_t left;

    left = m_buffer_size - m_buffer_offset_on;
    copy = MIN(left, bytes);
    memcpy(to + m_buffer_offset_on, from, copy);
    bytes -= copy;
    copied += copy;
    from += copy;
    m_buffer_offset_on += copy;
    if (m_buffer_offset_on >= m_buffer_size) {
      m_buffer_offset_on = 0;
      filled_audio_buffer(m_buffer_ts, resync | m_load_audio_do_next_resync);
      m_buffer_ts += m_msec_per_frame;
      resync = 0;
      m_load_audio_do_next_resync = 0;
    }
  }
  return;
}

void CPDPAudioSync::filled_audio_buffer (uint64_t ts, int resync)
{
  uint32_t fill_index;
  int locked;
  // m_dont_fill will be set when we have a pause
  if (m_dont_fill == 1) {
    return;
  }
  //  resync = 0;
  fill_index = m_fill_index;
  m_fill_index++;
  m_fill_index %= DECODE_BUFFERS_MAX;

  locked = 0;
  if (m_audio_initialized != 0) {
    locked = 1;
  }
  if (m_first_filled != 0) {
    m_first_filled = 0;
    resync = 0;
    m_resync_required = 0;
  } else {
    int64_t diff;
    diff = ts - m_last_fill_timestamp;
    if (diff - m_msec_per_frame > m_msec_per_frame) {
      // have a hole here - don't want to resync
      if (diff > ((m_msec_per_frame + 1) * 4)) {
	resync = 1;
      } else {
	// try to fill the holes
	m_last_fill_timestamp += m_msec_per_frame + 1; // fill plus extra
	int64_t ts_diff;
	do {
	  uint8_t *retbuffer;
	  // Get and swap buffers.
	  retbuffer = get_audio_buffer();
	  if (retbuffer == NULL) {
	    return;
	  }
	  if (retbuffer != m_sample_buffer[m_fill_index]) {
	    audio_message(LOG_ERR, "retbuffer not fill index in audio sync");
	    return;
	  }
	  locked = 0;
	  if (m_audio_initialized != 0) {
	    locked = 1;
	  }
	  m_sample_buffer[m_fill_index] = m_sample_buffer[fill_index];
	  m_sample_buffer[fill_index] = retbuffer;
	  memset(retbuffer, m_silence, m_buffer_size);
	  m_buffer_time[fill_index] = m_last_fill_timestamp;
	  m_buffer_filled[fill_index] = 1;
	  m_samples_loaded += m_buffer_size;
	  fill_index++;
	  fill_index %= DECODE_BUFFERS_MAX;
	  m_fill_index++;
	  m_fill_index %= DECODE_BUFFERS_MAX;
	  audio_message(LOG_NOTICE, "Filling timestamp %llu with silence",
			m_last_fill_timestamp);
	  m_last_fill_timestamp += m_msec_per_frame + 1; // fill plus extra
	  ts_diff = ts - m_last_fill_timestamp;
	  audio_message(LOG_DEBUG, "diff is %lld", ts_diff);
	} while (ts_diff > 0);
	locked = 0;
	if (m_audio_initialized != 0) {
	  locked = 1;
	}
      }
    } else {
      if (m_last_fill_timestamp == ts) {
	audio_message(LOG_NOTICE, "Repeat timestamp with audio %llu", ts);
	return;
      }
    }
  }
  m_last_fill_timestamp = ts;
  m_buffer_filled[fill_index] = 1;
  m_samples_loaded += m_buffer_size;
  m_buffer_time[fill_index] = ts;
  if (resync) {
    m_resync_required = 1;
    m_resync_buffer = fill_index;
#ifdef DEBUG_AUDIO_FILL
    audio_message(LOG_DEBUG, "Resync from filled_audio_buffer");
#endif
  }

  // Check this - we might not want to do this unless we're resyncing
  if (resync) m_psptr->wake_sync_thread();
#ifdef DEBUG_AUDIO_FILL
  audio_message(LOG_DEBUG, "Filling " LLU " %u %u", ts, fill_index, m_samples_loaded);
#endif
}

void CPDPAudioSync::set_eof(void) 
{ 
  uint8_t *to;
  if (m_buffer_offset_on != 0) {
    to = get_audio_buffer();
    if (to != NULL) {
      uint32_t left;
      left = m_buffer_size - m_buffer_offset_on;
      memset(to + m_buffer_offset_on, 0, left);
      m_buffer_offset_on = 0;
      filled_audio_buffer(m_buffer_ts, 0);
      m_buffer_ts += m_msec_per_frame;
    }
  }
  CAudioSync::set_eof();
}

int CPDPAudioSync::initialize_audio (int have_video) 
{
  return (1);
}

int CPDPAudioSync::is_audio_ready (uint64_t &disptime)
{
  disptime = m_buffer_time[m_play_index];
  return (m_dont_fill == 0 && m_buffer_filled[m_play_index] == 1);
}

uint64_t CPDPAudioSync::check_audio_sync (uint64_t current_time, int &have_eof)
{
  return (0);
}

void CPDPAudioSync::audio_callback (Uint8 *stream, int ilen)
{
  int freed_buffer = 0;
  uint32_t bufferBytes = (uint32_t)ilen;
  uint64_t this_time;
  int delay = 0;
  int playtime;

}

void CPDPAudioSync::play_audio (void)
{
  m_first_time = 1;
  m_audio_paused = 0;
  m_play_sample_index = 0;
}

void CPDPAudioSync::flush_sync_buffers (void)
{
  clear_eof();
  m_dont_fill = 1;
  if (m_audio_waiting_buffer) {
    m_audio_waiting_buffer = 0;
  }
}

void CPDPAudioSync::flush_decode_buffers (void)
{
  int locked = 0;
  if (m_audio_initialized != 0) {
    locked = 1;
  }
  m_dont_fill = 0;
  m_first_filled = 1;
  for (int ix = 0; ix < DECODE_BUFFERS_MAX; ix++) {
    m_buffer_filled[ix] = 0;
  }
  m_buffer_offset_on = 0;
  m_play_index = m_fill_index = 0;
  m_audio_paused = 1;
  m_resync_buffer = 0;
  m_samples_loaded = 0;
}

void CPDPAudioSync::set_volume (int volume)
{
  m_volume = (volume * SDL_MIX_MAXVOLUME)/100;
}

void CPDPAudioSync::audio_convert_data (void *from, uint32_t samples)
{
  if (m_obtained.format == AUDIO_U8 || m_obtained.format == AUDIO_S8) {
    // bytewise - easy
    int8_t *src, *dst;
    src = (int8_t *) from;
    dst = (int8_t *) m_convert_buffer;
    if (m_channels == 2) {
      // we got 1, wanted 2
      for (uint32_t ix = 0; ix < samples; ix++) {
	int16_t sum = *src++;
	sum += *src++;
	sum /= 2;
	if (sum < -128) sum = -128;
	else if (sum > 128) sum = 128;
	*dst++ = sum & 0xff;
      }
    } else {
      // we got 2, wanted 1
      for (uint32_t ix = 0; ix < samples; ix++) {
	*dst++ = *src;
	*dst++ = *src++;
      }
    }
  } else {
    int16_t *src, *dst;
    src = (int16_t *) from;
    dst = (int16_t *) m_convert_buffer;
    samples /= 2;
    if (m_channels == 1) {
      // 1 channel to 2
      for (uint32_t ix = 0; ix < samples; ix++) {
	*dst++ = *src;
	*dst++ = *src;
	src++;
      }
    } else {
      // 2 channels to 1
      for (uint32_t ix = 0; ix < samples; ix++) {
	int32_t sum = *src++;
	sum += *src++;
	sum /= 2;
	if (sum < -32768) sum = -32768;
	else if (sum > 32767) sum = 32767;
	*dst++ = sum & 0xffff;
      }
    }

  }
}

static void pdp_audio_config (void *ifptr, int freq, 
			    int chans, int format, uint32_t max_buffer_size)
{
  ((CPDPAudioSync *)ifptr)->set_config(freq,
				    chans,
				    format,
				    max_buffer_size);
}

static uint8_t *pdp_get_audio_buffer (void *ifptr)
{
  return ((CPDPAudioSync *)ifptr)->get_audio_buffer();
}

static void pdp_filled_audio_buffer (void *ifptr,
				   uint64_t ts,
				   int resync_req)
{
  ((CPDPAudioSync *)ifptr)->filled_audio_buffer(ts, 
					     resync_req);
}

static void pdp_load_audio_buffer (void *ifptr, 
				     uint8_t *from, 
				     uint32_t bytes, 
				     uint64_t ts, 
				     int resync)
{
  ((CPDPAudioSync *)ifptr)->load_audio_buffer(from,
					      bytes,
					      ts, 
					      resync);
}
  
audio_vft_t audio_vft = {
  message,
  pdp_audio_config,
  pdp_get_audio_buffer,
  pdp_filled_audio_buffer,
  pdp_load_audio_buffer
};

audio_vft_t *get_audio_vft (void)
{
  return &audio_vft;
}

CPDPAudioSync *pdp_create_audio_sync (CPlayerSession *psptr, t_pdp_mp4player *pdp_father)
{
  return new CPDPAudioSync(psptr, pdp_father);
}

int do_we_have_audio (void) 
{
  return 1;
}
