/***********************************************
v1.0.0:
	release init camera hal version.
v1.1.0: 
	1. fix some ae bugs
	2. fix manual isp bugs
v1.2.0:
	1. support dump RAW file.
v1.3.0:
	1. isp tunning tool function has been verified.
	2. optimize aec algorithm.
	3. disable lots of info logs.
v1.4.0:
	1. ov2710 & ov4689 optimized IQ
v1.5.0:
	1. hst weight and flt setting are not expected,fix it
	2. update ov4689 xml
v1.6.0:
	1. fix get metadata bug
	2. fix mapSensorExpToHal bug
	3. add imx323 tunning file
	4. fix ae bugs
	5. wdr & adpf can config in tunning file 
v1.7.0:
	1. ov2710 & ov4689 WDR and adpf off, GOC on
v1.8.0:
	1. fix calibdb memory leak
	2. CameraBuffer support timestamp
	3. update im3x3 tunning file
v1.9.0:
	1. can set  wb & fliker mode & ae bias 
	   through user interface
	2. ae was stopped for ov4689 under paticular condition,
	   fix it.
v1.a.0:
	1. imx323,ov4689,ov2710 wdr on,goc off
v1.b.0:
	1. use isp_sensor_output_width instead of 
	   isp_sensor_input_width	
v1.c.0:
	1. use different goc curve for wdr on or off
	2. auto detect isp,cif,usb video devices
v1.d.0:
	1.
        IQ: imx323 v0.3.0
            Gamma on.
            WDR on.
            saturation set at [80 80 80 70]
            ov4689 v0.4.0
	2. support isp driver v0.1.7
v1.e.0:
	1.

	    AEC: Add AOE algorithm and AOE parameter parse.
		 -----------------------------------
	    IQ:  imx323 v0.3.1
		 Gamma on;Gamma curve change.
		 WDR on.
		 CC offset set at [0 0 0].
		 CC sat set at [100 100 100 80].
		 add AOE parameter but still can not enable.
		 -----------------------------------
		 ov4689 v0.4.1
		 Gamma on;Gamma curve change.
		 WDR on.
		 CC offset set at [0 0 0].
		 CC sat set at [100 100 100 90].
		 add AOE parameter but still can not enable.
	2. fix calibdb memory leak
v1.e.1:
	1. initial awb mode doesn't take effect,fix it
	2. fix adpf deNoiseLevel pointer bug
	3. sensor connected to isp adapter support fps control
v1.e.2:
	1. add 3A alogrithm result to medta data
v1.e.3:
	1. add open flag O_CLOEXEC for device
	2. support awdr
	3. select IQ file from /tmp/ prior to /etc/cam_iq
	4. update IQ file: imx323 v0.3.5 
			   ov4689 v0.4.9
		
***********************************************/
#define CAMHALVERSION "1.e.3"
