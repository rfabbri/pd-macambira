/*
	$Id: dc1394.c,v 1.1 2005-10-04 02:02:15 matju Exp $

	GridFlow
	Copyright (c) 2001,2002,2003,2004 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>
#include "../base/grid.h.fcs"

typedef raw1394handle_t RH;
typedef nodeid_t NID;

static const int ruby_lineno = __LINE__;
static const char *ruby_code =
\ruby

def choice(*)end
class CStruct
  def initialize(*) end
end
	
choice :name,:Speed,:start,0,:values,%w(SPEED_100 SPEED_200 SPEED_400)
choice :name,:Framerate,:start,32,:values,
%w(FRAMERATE_1_875 FRAMERATE_3_75 FRAMERATE_7_5 FRAMERATE_15
   FRAMERATE_30 FRAMERATE_60)

choice :name,:Format0Mode,:start,64,:values,%w(
  MODE_160x120_YUV444 MODE_320x240_YUV422
  MODE_640x480_YUV411 MODE_640x480_YUV422
  MODE_640x480_RGB    MODE_640x480_MONO
  MODE_640x480_MONO16)

choice :name,:Format1Mode,:start,96,:values,%w(
  MODE_800x600_YUV422 MODE_800x600_RGB
  MODE_800x600_MONO   MODE_1024x768_YUV422
  MODE_1024x768_RGB   MODE_1024x768_MONO
  MODE_800x600_MONO16 MODE_1024x768_MONO16)

choice :name,:Format2Mode,:start,128,:values,%w(
  MODE_1280x960_YUV422 MODE_1280x960_RGB
  MODE_1280x960_MONO   MODE_1600x1200_YUV422
  MODE_1600x1200_RGB   MODE_1600x1200_MONO
  MODE_1280x960_MONO16 MODE_1600x1200_MONO16)

choice :name,:Format6Mode,:start,256,:values,%w(MODE_EXIF)

choice :name,:Format7Mode,:start,288,:values,%w(
  MODE_FORMAT7_0 MODE_FORMAT7_1 MODE_FORMAT7_2 MODE_FORMAT7_3
  MODE_FORMAT7_4 MODE_FORMAT7_5 MODE_FORMAT7_6 MODE_FORMAT7_7)

choice :name,:Format7ColorMode,:start,320,:values,%w(
  COLOR_FORMAT7_MONO8  COLOR_FORMAT7_YUV411
  COLOR_FORMAT7_YUV422 COLOR_FORMAT7_YUV444
  COLOR_FORMAT7_RGB8   COLOR_FORMAT7_MONO16
  COLOR_FORMAT7_RGB16)

choice :name,:TriggerMode,:start,352,:values,%w(
  TRIGGER_MODE_0 TRIGGER_MODE_1 TRIGGER_MODE_2 TRIGGER_MODE_3)

choice :name,:CameraImageFormat,:start,384,:values,%w(
  FORMAT_VGA_NONCOMPRESSED    FORMAT_SVGA_NONCOMPRESSED_1
  FORMAT_SVGA_NONCOMPRESSED_2 skip 3
  FORMAT_STILL_IMAGE          FORMAT_SCALABLE_IMAGE_SIZE)

choice :name,:CameraFeatures,:start,416,:values,%w(
  FEATURE_BRIGHTNESS FEATURE_EXPOSURE
  FEATURE_SHARPNESS  FEATURE_WHITE_BALANCE
  FEATURE_HUE        FEATURE_SATURATION
  FEATURE_GAMMA      FEATURE_SHUTTER
  FEATURE_GAIN       FEATURE_IRIS
  FEATURE_FOCUS      FEATURE_TEMPERATURE
  FEATURE_TRIGGER    skip 19
  FEATURE_ZOOM       FEATURE_PAN
  FEATURE_TILT       FEATURE_OPTICAL_FILTER
  skip 12            FEATURE_CAPTURE_SIZE
  FEATURE_CAPTURE_QUALITY skip 14)

choice :name,:DCBool,:start,0,:values,%w(False True)

#define MAX_CHARS 32
#define SUCCESS 1
#define FAILURE -1
#define NO_CAMERA 0xffff

# Parameter flags for setup_format7_capture()
#define QUERY_FROM_CAMERA -1
#define USE_MAX_AVAIL     -2
#define USE_RECOMMENDED   -3

# all dc1394_ prefixes removed
# raw1394handle_t = RH
# nodeid_t = NID
# RH+NID = RN

CameraInfo = CStruct.new %{
    RH rh;
    NID id;
    octlet_t ccr_offset;
    u_int64_t euid_64;
    char vendor[MAX_CHARS + 1];
    char model[MAX_CHARS + 1];
}

CameraCapture = CStruct.new %{
    NID node;
    int channel, frame_rate, frame_width, frame_height;
    int * capture_buffer;
    int quadlets_per_frame, quadlets_per_packet;
    const unsigned char * dma_ring_buffer;
    int dma_buffer_size, dma_frame_size, num_dma_buffers, dma_last_buffer;
    const char * dma_device_file;
    int dma_fd, port;
    struct timeval filltime;
    int dma_extra_count;
    unsigned char * dma_extra_buffer;
    int drop_frames;
}

MiscInfo = CStruct.new %{
  int format, mode, framerate;
  bool is_iso_on;
  int iso_channel, iso_speed, mem_channel_number;
  int save_channel, load_channel;
}

FeatureInfo = CStruct.new %{
    uint feature_id;
    bool available, one_push, readout_capable, on_off_capable;
    bool auto_capable, manual_capable, polarity_capable, one_push_active;
    bool is_on, auto_active;
    char trigger_mode_capable_mask;
    int trigger_mode;
    bool trigger_polarity;
    int min, max, value, BU_value, RV_value, target_value;
}

FeatureSet = CStruct.new %{
  FeatureInfo feature[NUM_FEATURES];
}
#void print_feature_set(FeatureSet *features);
#extern const char *feature_desc[NUM_FEATURES];
#void print_feature(FeatureInfo *feature);
#RawFire create_handle(int port);
#destroy_handle(RH rh);
#void print_camera_info(camerainfo *info);

class RH; %{
  # those two:
  # Return -1 in numCameras and NULL from the call if there is a problem
  # otherwise the number of cameras and the NID array from the call
  NID* get_camera_nodes(int *numCameras, int showCameras);
  NID* get_sorted_camera_nodes(int numids, int *ids, int *numCameras, int showCameras);
  release_camera(CameraCapture *camera);
  single_capture(CameraCapture *camera);
# this one returns FAILURE or SUCCESS
  multi_capture(CameraCapture *cams, int num);
}end

class RN; %{
  init_camera();
  is_camera(bool *value);
  get_camera_feature_set(FeatureSetFeatureInfo *features);
  get_camera_feature(FeatureInfo *feature);
  get_camera_misc_info(miscinfo *info);
  get_sw_version(quadlet_t *value);
  get_camera_info(camerainfo *info);
  query_supported_formats(quadlet_t *value);
  query_supported_modes(uint format, quadlet_t *value);
  query_supported_framerates(uint format, uint mode, quadlet_t *value);
  query_revision(int mode, quadlet_t *value);
  query_basic_functionality(quadlet_t *value);
  query_advanced_feature_offset(quadlet_t *value);
  attr set_video_framerate(uint framerate);
  attr video_mode(uint mode);
  attr video_format(uint format);
  double_attr iso_channel_and_speed(uint channel, uint speed);
  camera_on();
  camera_off();
  start_iso_transmission();
  stop_iso_transmission();
  get_iso_status(bool *is_on);
  set_one_shot();
  unset_one_shot();
  set_multi_shot(uint numFrames);
  unset_multi_shot();
# attributes :
# those are get_/set_ methods where the get has an input parameter
# and the set has an output parameter
  attr uint brightness
  attr uint exposure
  attr uint sharpness
  double_attr set_white_balance(uint u_b_value, uint v_r_value);
  attr uint hue
  attr uint saturation(uint saturation);
  attr uint gamma(uint gamma);
  attr shutter(uint shutter);
  attr uint gain
  attr uint iris
  attr uint focus
  attr uint trigger_mode
  attr uint zoom
  attr uint pan
  attr uint tilt
  attr uint optical_filter
  attr uint capture_size
  attr uint capture_quality
  int get_temperature(uint *target_temperature, uint *temperature);
  int set_temperature(uint target_temperature);

  get_memory_load_ch(uint *channel);
  get_memory_save_ch(uint *channel);
  is_memory_save_in_operation(bool *value);
  set_memory_save_ch(uint channel);
  memory_save();
  memory_load(uint channel);
  attr bool trigger_polarity
  trigger_has_polarity(bool *polarity);
  attr bool set_trigger_on_off

# this one returns SUCCESS on success, FAILURE otherwise
  setup_capture(
    int channel, int format, int mode, int speed, int frame_rate,
    CameraCapture *camera);
  dma_setup_capture(
    int channel, int format, int mode, int speed, int frame_rate, 
    int num_dma_buffers, int drop_frames, const char *dma_device_file,
    CameraCapture *camera);
  setup_format7_capture(
    int channel, int mode, int speed, int bytes_per_packet,
    uint left, uint top, uint width, uint height, CameraCapture *camera);
  dma_setup_format7_capture(
    int channel, int mode, int speed, int bytes_per_packet,
    uint left, uint top, uint width, uint height,
    int num_dma_buffers, CameraCapture *camera);
}end

#RNF = RN+uint feature
class RNF; %{
  query_feature_control(uint *availability);
  query_feature_characteristics(quadlet_t *value);
  attr uint feature_value
  is_feature_present(bool *value);
  has_one_push_auto(bool *value);
  is_one_push_in_operation(bool *value);
  start_one_push_operation();
  can_read_out(bool *value);
  can_turn_on_off(bool *value);
  is_feature_on(bool *value);
  feature_on_off(uint value);
  has_auto_mode(bool *value);
  has_manual_mode(bool *value);
  is_feature_auto(bool *value);
  auto_on_off(uint value);
  get_min_value(uint *value);
  get_max_value(uint *value);
}end

# DMA Capture Functions 
#dma_release_camera(RH rh, CameraCapture *camera);
#dma_unlisten(RH rh, CameraCapture *camera);
#dma_single_capture(CameraCapture *camera);
#dma_multi_capture(CameraCapture *cams,int num);
#dma_done_with_buffer(CameraCapture * camera);

# default return type is int, prolly means SUCCESS/FAILURE

#RNM = RN+uint mode
class RNM; %{
  query_format7_max_image_size(uint *horizontal_size, uint *vertical_size);
  query_format7_unit_size(uint *horizontal_unit, uint *vertical_unit);
  query_format7_color_coding(quadlet_t *value);
  query_format7_pixel_number(uint *pixnum);
  query_format7_total_bytes(uint *total_bytes);
  query_format7_packet_para(uint *min_bytes, uint *max_bytes);
  query_format7_recommended_byte_per_packet(uint *bpp);
  query_format7_packet_per_frame(uint *ppf);
  query_format7_unit_position(uint *horizontal_pos, uint *vertical_pos);
  # those were query/set pairs.
  double_qattr format7_image_position(uint left, uint top);
  double_qattr format7_image_size(uint width, uint height);
  qattr uint format7_color_coding_id
  qattr uint format7_byte_per_packet
  query_format7_value_setting(uint *present, uint *setting1, uint *err_flag1, uint *err_flag2);
  set_format7_value_setting(); //huh?
}end

\end ruby
;

\class FormatDC1394 < Format
struct FormatDC1394 : Format {
	\decl void initialize (Symbol mode);
	\decl void frame ();
};

\def void initialize(Symbol mode) {
	gfpost("DC1394: hello world");
	RH rh = raw1394_new_handle();
	int numPorts = raw1394_get_port_info(rh,0,0);
	raw1394_destroy_handle(rh);
	gfpost("there are %d Feuerweuer ports",numPorts);
	if (mode!=SYM(in)) RAISE("sorry, read-only");
	for(int port=0; port<numPorts; port++) {
		gfpost("trying port #%d...",port);
		RH rh = dc1394_create_handle(port);
		int numCameras=0xDEADBEEF;
		NID *nodes = dc1394_get_camera_nodes(rh,&numCameras,0);
		gfpost("port #%d has %d cameras",port,numCameras);
		for (int i=0; i<numCameras; i++) gfpost("camera at node #%d",nodes[i]);
		// I'm stuck here, can't find that iSight camera. -- matju
	}
	dc1394_destroy_handle(rh);
}

\def void frame () {
	gfpost("i'd like to get a frame from the cam, but how?");
}

\classinfo {
	IEVAL(rself,"install '#io:dc1394',1,1;@flags=4;@comment='Video4linux 1.x'");
	//IEVAL(rself,ruby_code);
	rb_funcall(rself,SI(instance_eval),3,rb_str_new2(ruby_code),
		rb_str_new2(__FILE__),INT2NUM(ruby_lineno+3));
}
\end class FormatDC1394
void startup_dc1394 () {
	\startall
}
