/*
 *
 * Display Controller Header file for TI 816X VPSS
 *
 * Copyright (C) 2009 TI
 * Author: Yihe Hu <yihehu@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA	02111-1307, USA.
 */

#ifndef __LINUX_VPS_DISPLAYCTRL_H__
#define __LINUX_VPS_DISPLAYCTRL_H__

#ifdef __KERNEL__
/*
 *  Display controller driver IOCTLs.
 */

/*
 *  Macros for display controller instance numbers to be passed as instance ID
 *  at the time of driver create.
 *  Note: These are read only macros. Don't modify the value of these macros.
 */
/** \brief Display controller instance 0. */
#define VPS_DCTRL_INST_0                    (0u)

/** \brief Command to set the entire VPS display path configuration in
 *  one shot.
 *
 *  All the VPS modules handled by the Display Controller can be
 *  represented by a graph, where node represents a module like blender,
 *  mux etc. and edge is present between two nodes if they are connected. All
 *  VPS paths can be configured by this IOCTL in one shot. Use macro
 *  defined in this file for input path, muxes, vcomp, cig input, cig
 *  output and blender as the node numbers.
 *
 *  This IOCTL takes either name of the pre-defined configuration or
 *  list of edges
 *  connecting nodes and configures display paths.
 *  It first validates these paths and then configures VPS for the display
 *  paths. It configures all the center modules, except blender. Blender will
 *  only be enabled when venc is configured with the given mode.
 *
 * \par this ioctl cannot be used for clearing configuration on a path
 *      streaming path.
 *
 * \param   cmdargs [in] pointer of type vps_dcconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_SET_CONFIG              (VPS_DCTRL_IOCTL_BASE + 0x1u)

/** \brief Command to clear the VPS display path configuration in
 *  one shot.
 *
 *  this ioctl takes either name of the pre-defined configuration or
 *  list of edges
 *  connecting nodes and clears the configuration. it also stops vencs.
 *
 *  It does not validates the edge list. It simply disables the edge
 *  connecting nodes. For the vencs, it checks for the validity and then
 *  disables the venc if there are not errors.
 *  Use macro defined in this file for input path, muxes, vcomp, cig input, cig
 *  output and blender as the node numbers.
 *
 * \par this ioctl cannot be used for clearing configuration on a path
 *      streaming path.
 *
 * \param   cmdargs [in] pointer of type vps_dcconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_CLEAR_CONFIG            (VPS_DCTRL_IOCTL_BASE + 0x2u)

/** \brief Command to set output in the given Venc.
 *
 *  This IOCTL is used to set the output i.e. composite, s-video etc. on
 *  the given venc if venc is not on. For the tied vencs, venc has to
 *  be stopped first, then output can can be changed.
 *
 * \param   cmdargs [in] pointer of type vps_dcoutputinfo
 *
 * \return  vps_sok if successful, else suitable error code
 *
 */
#define IOCTL_VPS_DCTRL_SET_VENC_OUTPUT         (VPS_DCTRL_IOCTL_BASE + 0x3u)

/** \brief Command to get output in the given Venc.
 *
 * \param   cmdargs [in] pointer of type vps_dcoutputinfo
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_GET_VENC_OUTPUT         (VPS_DCTRL_IOCTL_BASE + 0x4u)

/** \brief maximum number of basic ioctl commands
 *
 *  marker used to denote the maximum number of basic ioctls supported
 *
 *  \par caution ensure that basic ioctl value does not execeed this value
 */
#define VPS_DCTRL_IOCTL_BASIC_MAX               (VPS_DCTRL_IOCTL_BASE + 0x9u)

/** \brief Maximum number of characters in the string for specifying
 * node name */
#define VPS_DC_MAX_OUTPUT_NAME            (20u)

/** \brief Maximum number of edges in the list of edges in Dc_config
 * structure */
#define VPS_DC_MAX_EDGES                (46u)

/* Following macros define bitmasks for the Vencs. Here, bitmasks are
   used for identifying Vencs so that tied vencs can be easily specified
   and configured. Two vencs, which uses same pixel clock and whose
   vsync are synchronized, can be tied together. SD Venc cannot
   be tied with any other venc since it supports only SD modes and HD
   vencs supports only HD Modes. Even if it cannot be tied, same
   mechanism of configuring VENC is used for the SD VENC. */

/** \brief Bitmask for HDMI VENC */
#define VPS_DC_VENC_HDMI                (0x1u)
/** \brief Bitmask for HDCOMP VENC */
#define VPS_DC_VENC_HDCOMP              (0x2u)
/** \brief Bitmask for DVO2 VENC */
#define VPS_DC_VENC_DVO2                (0x4u)
/** \brief Bitmask for SD VENC */
#define VPS_DC_VENC_SD                  (0x8u)

/** \brief Defines maximum number of venc info structure, which can be
  * passed in setconfig API
  **/
#define VPS_DC_MAX_VENC                 (4u)

/** \brief on-chip encoder identifier - hdmi */
#define VPS_DC_ENCODER_HDMI                     (0x1u)
/** \brief on-chip encoder identifier - rf */
#define VPS_DC_ENCODER_RF                       (0x2u)

/** \brief on-chip encoder identifier - max guard */
#define VPS_DC_MAX_ENCODER                      (0x3u)

/**
 *  enum vps_dcdvo2clksrc
 *  \brief Enumerations for selecting DVO2 clock source.
 *  DVO2 clock can come from either HDMI or from HDCOMP VENC. Using this,
 *  application can select the clock source for DVO2 clock. Note that
 *  DVO2 clock source can be set only in the IOCTL IOCTL_VPS_DCTRL_SET_CONFIG.
 */
enum vps_dcdvo2clksrc {
	VPS_DC_DVO2CLKSRC_HDMI = 0,
	/**< DVO 2 uses clk from HDMI */
	VPS_DC_DVO2CLKSRC_HDCOMP
	/**< DVO 2 uses clk from HD VENC */
};
/**
 *  enum vps_dchdcompclksrc
 *  \brief Enumerations for selecting HDCOMP clock source. As such,
 *  HDCOMP (HDVENCA) has an independent pixel clock, but it can use
 *  HDMI pixel clock as well. This enum is used for selecting HDCOMP
 *  pixel clock source. Note that
 *  HDCOMP clock source can be set only in the
 *  IOCTL IOCTL_VPS_DCTRL_SET_CONFIG.
 */
enum vps_dchdcompclksrc {
    VPS_DC_HDCOMPCLKSRC_HDMI = 0,
    /**< DVO 2 uses clk from HDMI */
    VPS_DC_HDCOMPCLKSRC_HDCOMP
    /**< DVO 2 uses clk from HD VENC */
};
/**
 *  enum vps_dcusecase
 *  \brief Enum for selecting VPS configuration for the specific use
 *  case or user defined use case.
 *  Display Controller supports few pre-defined configurations. Pre-defined
 *  configurations configures all vps modules, including vencs, handled by
 *  display controller. Once a
 *  pre-defined configuration is used, all other parameters will be ignored
 *  in the Vps_DcConfig structure and display controller will be
 *  configured as per the pre-defined configuration.
 */
enum vps_dcusecase {
	VPS_DC_TRIDISPLAY = 0,
	/**< TRIDISPLAY configuration: Pre-defined configuration in which
	     HDMI, HDVENCA and SDVENC are used to provide three outputs. Both
	     the HDVENCs are running 1080p mode at 60 fps and SDVENC is
	     running NTSC mode. Private
	     path0 is connected to HDMI output through VCOMP, Private path1
	     is connected to HDVENCA throug CIG and transcode path is connected
	     to SDVENC. */
	VPS_DC_DUALHDDISPLAY,
	/**< DUALDISPLAY configuration: Pre-defined configuration in which
	     HDMI and DVO2 are used to provide two HD outputs. Both
	     the HDVENCs are running 1080p mode at 60 fps. Private
	     path0 is connected to HDMI output through VCOMP and Private path1
	     is connected to HDVENCA throug CIG. */
	VPS_DC_DUALHDSDDISPLAY,
	/**< DUALDISPLAY configuration: Pre-defined configuration in which
	     HDMI and SDVENC are used to provide one HD and one SD output.
	     HDVENC is running 1080p mode at 60 fps and SD VENC is running
	     PAL mode. Private path0 is connected to HDMI output through VCOMP
	     and Private path1 is connected to SDVENC. */
	VPS_DC_USERSETTINGS,
	/**< User Defined paths for VPS */
	VPS_DC_NUM_USECASE
	/**< This must be the last Enum */
};


/**
 * \brief enum vps_dcmodeid
 *  Enum defining ID of the standard Modes. Standard timinig parameters
 *  will be used if the standard mode id is used for configuring mode
 *  in the venc.
 */
enum vps_dcmodeid {
	VPS_DC_MODE_NTSC = 0,
	/**< Mode Id for NTSC */
	VPS_DC_MODE_PAL,
	/**< Mode Id for PAL */
	VPS_DC_MODE_1080P_60,
	/**< Mode Id for 1080p at 60fps mode */
	VPS_DC_MODE_720P_60,
	/**< Mode Id for 720p at 60fps mode */
	VPS_DC_MODE_1080I_60,
	/**< Mode Id for 1080I at 60fps mode */
	VPS_DC_MAX_MODE
	/**< This should be the last mode id */
};


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \brief dvo format
 */
enum vpshal_dcdigitalfmt {
	VPS_DC_D_OUTPUT_DVO = 0,
	/**< Digital output with 30 bit output discrete sync */
	VPS_DC_D_OUTPUT_HDMI,
	/**< Digital output with 30 bit output discrete sync enable HDMI */
	VPS_DC_D_OUTPUT_MAX
	/**< Max number of output supported */
};

/**
 * \brief analog format
 */
enum vpshal_dcanalogfmt {
	VPS_DC_A_OUTPUT_COMPOSITE = 0,
	/**< Analog output format composite */
	VPS_DC_A_OUTPUT_SVIDEO,
	/**< Analog output format svideo */
	VPS_DC_A_OUTPUT_COMPONENT,
	/**< Analog output format component */
	VPS_DC_A_OUTPUT_MAX
};

/**
 * \brief structure containing output information. this structure is used
 *  to set output in the output node. outputs are like composite,
 *  component etc.. this structure is used as an argument to
 *  ioctl_vps_dctrl_set_venc_output ioctl.
 */
struct vps_dcoutputinfo {
	u32			vencnodenum;
	/**< node number of the venc */
	u32			dvofmt;
	/**< digital output  */
	u32			afmt;
	/**< analog output */
};

/**
 * \brief Structure containing edge information. Edge is a connection
 *	between two nodes i.e. two modules (like CIG and Blend) in VPS.
 *	VPS can be represented by a Directed Acyclic graph, where each
 *	module is node and edge is present between two nodes if they
 *	are connected.
 *	All VPS paths can be configured in one shot by API vps_dcsetConfig().
 *	This api takes the array of edges connected between nodes.
 *	This structure is used to specify individual edge information.
 */
struct vps_dcedgeinfo {
	u32			startnode;
	/**< starting node (vps module) of the edge */
	u32			endnode;
	/**< End node (VPS Module) of the edge */
} ;
/**
 *  struct Vps_DcTimingInfo
 *  \brief Structure for configuring the HDVenc timing parameters.
 */
struct vps_dctiminginfo {
	u32			pixelclock;
	/**< Required pixel clock */
	u32			hfrontporch;
	/**< Horizontal front porch. Same for both fields in case of interlaced
	display */
	u32			hbackporch;
	/**< Horizontal back porch */
	u32			hsynclen;
	/**< Horizontal sync length. Same for both fields in case of interlaced
	display */
	u32			vfrontporch[2];
	/**< Vertical front porch for each field or frame */
	u32			vbackporch[2];
	/**< Vertical back porch for each field or frame */
	u32			vsynclen[2];
	/**< Vertical sync length for each field */
	u32			height;
	/**< Active height of each field or frame */
	u32			width;
	/**< Active width for each frame. Same for both fields in case
	     of interlaced display */
	u32			scanformat;
	/**< Scan format */
	u32			mode;
	/**< mode */

};

/**
 * \brief Structure containing mode information.
 */
struct vps_dcmodeinfo {
	u32              vencid;
	/**< Identifies the VENC on which mode parameters is to be applied.
	     Use one of VPS_DC_VENC_HDMI, VPS_DC_VENC_HDCOMP,
	     VPS_DC_VENC_DVO2, VPS_DC_VENC_SD macro for this variable. */
	u32             iscustommode;
	/**< Flag to indicate whether custom mode is used or not.
	     Display controller provides two ways to configure mode in the
	     venc i.e. by specifying mode in the modeid
	     variable where standard timing parameters will be used for
	     the mode and by specifying all the timing parameters for the mode.
	     For the standard mode, only modeId variables needs to be
	     configured.
	     Note: Custom timing parameters are still not supported in
	     the display controller */
	u32              modeid;
	/**< Id of the mode. Currently display controller supports 1080p-60
	     on the HDVENCs and NTSC/PAL on the SDVENC */
	u32              framewidth;
	/**< Width of the frame. Used only for the custom mode, which is
	     not supported currently */
	u32              frameheight;
	/**< Height of the frame. Used only for the custom mode, which is
	     not supported currently */
	u32              scanformat;
	/**< scan format. used only for the custom mode, which is
	     not supported currently. for valid values see #fvid2_scanformat.
	     */
	struct	vps_dctiminginfo     *tinfo;
	/**< timing information.  if this is null this will be used to configure
	     the timings instead of the standard modeid. currently this is not
	     supported */
};

/**
 * struct vps_dcvencinfo
 * \brief Structure containing venc information. This structure is used is
 *	vps_dcsetConfig API to configure mode in the Vencs. It also is used to
 *	inform which vencs are tied. Two vencs are tied when both are running
 *	on the same pixel clock speed and vsync signal for both the vencs are
 *	synchronized. If two vencs are tied, same mode is set in both the vencs
 *	and	they will be enabled at the same time in order to have both
 *	synchronized.
 */
struct vps_dcvencinfo {
	struct        vps_dcmodeinfo modeinfo[VPS_DC_MAX_VENC];
	/**< Mode Information to be set the Venc. */
	u32           tiedvencs;
	/**< bitmask of tied vencs. two vencs, which uses same pixel clock
	     and whose vsync are synchronized, can be tied together. */
	u32          numvencs;
	/**< Name of the mode to be set the Vencs */
};

/**
 * struct vps_dcconfig
 * \brief Structure contaning set of edges for creating complete VPS
 *	connection mesh and VENC information. This structure is used in
 *	vps_dcsetConfig API to configure Complete VPS mesh statically in
 *	one shot.
 *	It has set of edges, which describes how individual modules are
 *	connected to each other and finally to the VENC. It also configures
 *	the mode in the	VENCs and tells which vencs are tied.
 */
struct vps_dcconfig {
	u32                          usecase;
	/**< Indicates which use case is to be configured for. Media
	     Controller provides set of standard configuration
	     for some standard use cases. Application can
	     directly If it is standard use case, there is no need
	     to specify next arguments. Application can also specify
	     user defined path configuration by specifying
	     VPS_DC_USERSETTINGS in this argument and providing list of
	     edges */
	struct vps_dcedgeinfo        edgeinfo[VPS_DC_MAX_EDGES];
	/**< List of edges connecting vps modules. Display controller parse
	     these edges and enables/disables input/output path in the
	     appropriate VPS module. This edge tells which module is
	     connected to which module enabling output in edge start module
	     and input in edge end module. */
	u32                         numedges;
	/**< Number edge in the edgeInfo array */
	u32                         dvo2clksrc;
	/**< Dvo2 uses pixel clock either from HDMI or from VENC_A. This
	     configures clock source for DVO2. */
	u32                         hdcompclksrc;
	 /**< HDCOMP (HDVENCA) uses pixel clock either from HDMI or from
	      independent clock. This configures clock source for HDCOMP. */
	struct vps_dcvencinfo       vencinfo;
	/**< Structure containing Venc Information like mode to be configured
	     and which are tied. */
};

/**
 *	struct vps_dcyuvcolor
 * \brief Structure containing color information in YCrCb could be used for
 *	background and alternate color.
 */
struct vps_dcyuvcolor {
	u16 yluma;
	/**< Luma/Y Value */
	u16 crchroma;
	/**< Chroma/Cr Value */
	u16 cbchroma;
	/**< Chroma/Cb Value */
};

/**
 *	struct vps_dcrgbcolor
 * \brief Structure containing color information in YCrCb could be used for
 *	background and alternate color.
 */
struct vps_dcrgbcolor {
	u16 r;
	/**< Red Value */
	u16 g;
	/**< Green Value */
	u16 b;
	/**< Blue Value */
};

/* ======================================================================== */
/* ======================================================================== */
/*                             RunTime Configuration                          */
/* ======================================================================== */
/* ======================================================================== */

/* ======================================================================== */
/*                           Macros & Typedefs                                */
/* ======================================================================== */


/** \brief start of runtime control
 *
 *  marker used to denote the begining of runtime control ioctls
 *
 *  \par caution ensure that basic ioctl value does not execeed this value
 */
#define VPS_DCTRL_IOCTL_RT_BASE             (VPS_DCTRL_IOCTL_BASIC_MAX + 0x1u)

/** \brief Command for setting Vcomp runtime configuration.
 *
 *  This IOCTL is used to set the runtime configuration in VCOMP. This
 *  configuration includes setting the priority of the input
 *  video window. This can be even changed when streaming is on on
 *  both the video windows.
 *
 *  This IOCTL takes pointer to the structure Vps_DcVcompRtConfig as
 *  an argument and return 0 on success or negative error code on failure. */
#define IOCTL_VPS_DCTRL_SET_VCOMP_RTCONFIG  (VPS_DCTRL_IOCTL_RT_BASE + 0x1u)

/** \brief command for getting vcomp runtime configuration.
 *
 *  this ioctl is used to get the runtime configuration from vcomp. this
 *  configuration includes the priority of the input
 *  video window.
 *
 * \param   cmdargs [in] pointer of type vps_dcvcomprtconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_GET_VCOMP_RTCONFIG  (VPS_DCTRL_IOCTL_RT_BASE + 0x2u)

/** \brief command for setting comp runtime configuration.
 *
 *  this ioctl is used to set the runtime configuration in comp. this
 *  configuration includes setting the priority of the input video
 *  window and graphics windows. it also specifies whether to use
 *  global or pixel base alpha blending. this can be even changed
 *  when streaming is on. specify nodeid of the comp in the
 *  vps_dccomprtconfig structure to set configuration in specific comp.
 *
 * \param   cmdargs [in] pointer of type vps_dccomprtconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_SET_COMP_RTCONFIG   (VPS_DCTRL_IOCTL_RT_BASE + 0x3u)

/** \brief command for getting comp runtime configuration.
 *
 *  this ioctl is used to get the runtime configuration from comp.
 *  specify nodeid of the comp in the vps_dccomprtconfig structure to
 *  get configuration in specific comp.
 *
 * \param   cmdargs [in] pointer of type vps_dccomprtconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_GET_COMP_RTCONFIG   (VPS_DCTRL_IOCTL_RT_BASE + 0x4u)

/** \brief command for setting cig runtime configuration.
 *
 *  this ioctl is used to set the runtime configuration in cig. this
 *  configuration includes alpha value
 *  for the blending and transparency color value for all three output video
 *  windwos. specify nodeid of the cig in the
 *  vps_dccigrtconfig structure to set configuration in specific cig output.
 *
 * \param   cmdargs [in] pointer of type vps_dccigrtconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_SET_CIG_RTCONFIG    (VPS_DCTRL_IOCTL_RT_BASE + 0x5u)

/** \brief command for getting cig runtime configuration.
 *
 *  this ioctl is used to get the runtime configuration in cig. specify
 *  nodeid of the cig in the vps_dccigrtconfig structure to set
 *  configuration in specific cig output.
 *
 * \param   cmdargs [in] pointer of type vps_dccigrtconfig
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_GET_CIG_RTCONFIG    (VPS_DCTRL_IOCTL_RT_BASE + 0x6u)

/** \brief maximum number of runtime ioctl commands
 *
 *  marker used to denote the maximum number of runtime ioctls supported
 *
 *  \par caution ensure that runtime ioctl value does not execeed this value
 */
#define VPS_DCTRL_IOCTL_RT_MAX              (VPS_DCTRL_IOCTL_RT_BASE + 0x9u)


/**
 *  enum vps_dccigtransmask
 *	\brief Enumerations for transparency color mask bit (Number of LSB
 *       bits to mask when checking for pixel transparency).
 */
enum vps_dccigtransmask {
	VPS_DC_CIG_TM_NO_MASK = 0,
	/**< Disable Masking */
	VPS_DC_CIG_TM_MASK_1_LSB,
	/**< Mask 1 LSB before checking */
	VPS_DC_CIG_TM_MASK_2_LSB,
	/**< Mask 2 LSB before checking */
	VPS_DC_CIG_TM_MASK_3_LSB
	/**< Mask 3 LSB before checking */
};

/**
 * \brief enum vps_dccompdisplayorder_t
 * Enum for display order selection. PLease note alpha blending is done from
 * bottom to top layer
 */
enum vps_dccompdisplayorder {
	VPS_DC_COMP_DISPLAY_VID_ORDER = 0,
	/* Video layer display order.
	From low to high: 00, 01, 10, and 11 */
	VPS_DC_COMP_DISPLAY_G1_ORDER,
	/* Graphic1 layer display order when g_reorder = 1.
	 From low to high: 00, 01, 10, and 11 */
	VPS_DC_COMP_DISPLAY_G2_ORDER,
	/* Graphic2 layer display order when g_reorder = 1.
	From low to high: 00, 01, 10, and 11 */
	VPS_DC_COMP_DISPLAY_G3_ORDER,
	/* Graphic3 layer display order when g_reorder = 1.
	From low to high: 00,	 01, 10, and 11 */
	VPS_DC_COMP_DISPLAY_ORDER_MAX
	/* Defined to find out maximum*/
};

/**
 * \brief enum vps_dccompfeedbkpathselect_t
 *  Enum for selection of Feedback data. The feedback video data can
 *  be selected from video alpha blending or final alpha blending. This
 *  enum is used for selecting source of the feedback path.
 */
enum vps_dccompfeedbkpathselect {
	VPS_DC_COMP_OUT_VIDEO_ALPHA_BLENDING = 0,
	/* select data from video alpha blending */
	VPS_DC_COMP_OUT_FINAL_ALPHA_BLENDING
	/* select data from final alpha blending */
};

/* ======================================================================== */
/*                         Structure Declarations                             */
/* ======================================================================== */

/**
 * struct vps_dcvcomprtconfig
 * \brief Structure containing runtime configurable parameters for VCOMP
 *	from the display controller. This just includes which of the input video
 *  window is on the top when compositing in the VCOMP. This structure is
 *  passed as an argument to the IOCTL IOCTL_VPS_DCTRL_SET_VCOMP_RTCONFIG.
 */
struct vps_dcvcomprtconfig {
	u32		isprimaryvideoontop;
	/**< DEI_HQ input video path is considered as the privide video path.
	     This flag indicates whether primary video window is on top or not.
	     If it is false, Aux video window will be on the top. */
};

/**
 * struct vps_dccigrtconfig
 * \brief Structure containing runtime configurable parameters for CIG
 *	from the display controller. This includes Blending and transparency
 *  for the output video windows. This structure is passed as an
 *  argument to IOCTL IOCTL_VPS_DCTRL_SET_CIG_RTCONFIG.
 */
struct vps_dccigrtconfig {
	u32				nodeid;
	/**< Id of the node. Use one of VPS_DC_CIG_CONSTRAINED_OUTPUT,
	     VPS_DC_CIG_NON_CONSTRAINED_OUTPUT, VPS_DC_CIG_PIP_OUTPUT as
	     node id for configuring CIG runtime configuration. */
	u32				transparency;
	/**< Enable Transparency */
	u32				mask;
	/**< Transparency color mask bit enum type*/
	u32				alphablending;
	/**< Enable alpha blending */
	u8				alphavalue;
	/**< Alpha blending value */
	struct vps_dcrgbcolor		transcolor;
	/**< Transparency color in RGB */
};

/**
 * struct vps_dccomprtconfig
 * \brief Structure containing runtime configurable parameters for COMP
 *	from the display controller. These parameters includes priority of the
 *	input layers and whether to use global reordering or not. For each of
 *	the comp this runtime configurable parameters can be set separately. For
 *	setting runtime parameters for all blenders, this ioctl should be called
 *      multiple times. This structure is passed as an
 *      argument to IOCTL IOCTL_VPS_DCTRL_SET_COMP_RTCONFIG
 */
struct vps_dccomprtconfig {
	u32                       nodeid;
	/**< id of the node. use one of vps_dc_hdmi_blend,
	vps_dc_hdcomp_blend, vps_dc_dvo2_blend,
	vps_dc_sdvenc_blend as node id for runtime configuration of comp */
	u32                       ispipbasevideo;
	/**< CIG PIP output goes to all threee HD Blenders. This flag indicates
	     whether this pip video is base video or not. */
	u32                       fbpath;
	/**< Selects Feedback data path. These selects the source of the
	     feedback path. Feedback path from the blender is
	     supported only for the DVO1 and DVO2 output in the hardware. There
	     is not feedback path from the other blenders, so these parameter
	     will not be used for these blender */
	u32                      isglobalreorderenable;
	/**< Enables/Disables global reordering. If global reordering is
	     enabled, Display order/priority for the input paths are specified
	     in the displayOrder member of this structure. Otherwise, only
	     display order of the video window is used from the displayOrder
	     array. Graphics will bring priority per pixel. */
	u32                     displayorder[VPS_DC_COMP_DISPLAY_ORDER_MAX];
	/**< decides order of priority between three graphic and video
	     if isGlobalReorderEnable is set TRUE. It is array of size 4 and
	     user has to configure priority.  In the case,if gReorderMode is
	     set 0, user has to provide priority for vidoe layer only and for
	     graphic layer, it will be taken from settings or bits [35:32]
	     in the data bus.*/
};
/* ========================================================================== */
/* ========================================================================== */
/*                             Advanced Configuration                         */
/* ========================================================================== */
/* ========================================================================== */

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/** \brief advanced configurations
 *
 *  marker used to denote the begining of ioctls that would be required for
 *  for advanced control
 *
 *  \par caution ensure that basic ioctl value does not execeed this value
 */
#define VPS_DCTRL_IOCTL_ADV_BASE    (VPS_DCTRL_IOCTL_RT_MAX + 0x1u)

/** \brief Command to enumerate VPS nodes and its information for the
 *  node available at the given index.
 *
 *  This IOCTL is used for enumerating and then dynamically connecting different
 *  VPS modules. Enumeration starts from the node number 0 and
 *  continues untill it returns negative error code. On each index, this IOCTL
 *  returns node and its information like type of node number, name, number of
 *  inputs and number output available at given index. Only nodes, in
 *  which inputs can be enabled/disabled, can be enumerated with this
 *  ioctl. It returns node number from macros defined for input path,
 *  muxes, vcomp, cig input, cig output and blender.
 *
 * \param   cmdargs [in/out] pointer of type vps_dcenumnode, which would be
 *                           updated with input node information. provided that
 *                           node index and input are valid.
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_ENUM_NODES          (VPS_DCTRL_IOCTL_ADV_BASE + 0x1u)

/** \brief Command to enumerate nodes connected as a inputs to the given node.
 *  It enumerates inputs nodes of this node.
 *
 *  To enumerate all inputs, applications shall begin with input
 *  index zero, get the information,
 *  increment by one until the driver returns -1.
 *
 * \param   cmdargs [in/out] pointer of type vps_dcenumnodeinput, which would be
 *                           updated with input node information. provided that
 *                           node index and input are valid.
 *
 * \return  vps_sok if successful, else suitable error code
 *
 */
#define IOCTL_VPS_DCTRL_ENUM_NODE_INPUTS    (VPS_DCTRL_IOCTL_ADV_BASE + 0x2u)

/** \brief command to enable/disable the given input on the given node
 *
 *  this ioctl enables or disables input at the given index
 *  on the given node. it enables/disables given input as inputs
 *  node to this node and enables/disables given node
 *  as output node to the parent node i.e. it enables/disables edge connecting
 *  given node and input node. for example,
 *  to enable/disable cig pip output on hdcomp blender, use nodeid as
 *  vps_dc_hdcomp_blend and vps_dc_cig_pip_output as inputnodeid. this
 *  will enable/disable cig pip output in the cig and cig pip input in the
 *  hdcomp blender.
 *
 * \par caution this ioctl can not be used to enable node input
 *       if streaming on that input.
 *
 * \param   cmdargs [in] pointer of type vps_dcnodeinput
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_NODE_INPUT          (VPS_DCTRL_IOCTL_ADV_BASE + 0x3u)

/** \brief command to get the status of the given input on the given node.
 *
 *  this command is used to get the status i.e. whether it is
 *  enabled or not, of the input on the given node.
 *
 *  for example, to get the status of cig pip on hdcomp blender, use nodeid as
 *  vps_dc_hdcomp_blend and vps_dc_cig_pip_output as inputnodeid. it
 *  will return the status of this input on the blender node.
 *
 * \param   cmdargs [in] pointer of type vps_dcnodeinput
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_GET_NODE_INPUT_STATUS  (VPS_DCTRL_IOCTL_ADV_BASE + 0x4u)

/** \brief Command to set mode/standard in the given Venc.
 *
 *  This IOCTL is used to set the mode/standard in the given vencs
 *  if its inputs are not running. If the multiple venc is tied, then
 *  it sets mode in all the vencs if their inputs are not running.
 *  Otherwise it returns error. The IOCTL first stops venc, changes
 *  mode and restarts venc. Note that this IOCTL can break tying of
 *  vencs if used incorrectly.
 *
 * \param   cmdargs [in] pointer of type vps_dcvencinfo
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_SET_VENC_MODE   (VPS_DCTRL_IOCTL_ADV_BASE + 0x5u)

/** \brief command to get current mode/standard set in the given venc.
 *
 * \param   cmdargs [in] pointer of type vps_dcvencinfo
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_GET_VENC_MODE   (VPS_DCTRL_IOCTL_ADV_BASE + 0x6u)

/** \brief command to stop streaming on the venc.
 *
 *  it stops streaming on the given venc or set of vencs.
 *  application should pass the bit mask of all the vencs, which
 *  needs to be stopped.
 *
 * \param   cmdargs [in] pointer of type u32, that contains bitmask of all
 *          the vencs to be disabled.
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_DISABLE_VENC    (VPS_DCTRL_IOCTL_ADV_BASE + 0x7u)

/** \brief Command for Selecting output clock source for the given venc.
 *
 *  This IOCTL is used to select output clock source for the given venc.
 *  All HD Vencs has two input clocks i.e. clk1x and clk2x. One of them
 *  can be selected as the output clock
 *
 * \param   cmdargs [in] pointer of type vps_dcoutputclksel
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_SEL_OUT_CLK     (VPS_DCTRL_IOCTL_ADV_BASE + 0x8u)

/** \brief command for selecting clk1x input source
 *
 *  there are two input clocks to hd vencs i.e. clk2x and clk1x. clk1x
 *  input can be either clk2x or half of the clk2x. this ioctl is
 *  used to select clk1x clock source.
 *
 * \param   cmdargs [in] pointer of type vps_dcvencclkdivsel
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_SEL_CLK1X_SRC   (VPS_DCTRL_IOCTL_ADV_BASE + 0x9u)


/** \brief maximum number of advanced ioctl commands
 *
 *  marker used to denote the maximum number of advance ioctls supported
 *
 *  \par caution ensure that advanced ioctl value does not execeed this value
 */
#define VPS_DCTRL_IOCTL_ADV_MAX         (VPS_DCTRL_IOCTL_ADV_BASE + 0x20u)

/* Macros, Which can be used in the setconfig API to connect different
 * modules */

/** \brief macro defining main input path */
#define VPS_DC_MAIN_INPUT_PATH              (0u)
/** \brief macro defining auxiliary input path */
#define VPS_DC_AUX_INPUT_PATH               (4u)
/** \brief macro defining bypass 0 input path */
#define VPS_DC_BP0_INPUT_PATH               (5u)
/** \brief macro defining bypass 1 input path */
#define VPS_DC_BP1_INPUT_PATH               (6u)
/** \brief macro defining transcode/secondary 2 input path */
#define VPS_DC_SEC1_INPUT_PATH              (10u)
/** \brief macro defining graphics 0 input path */
#define VPS_DC_GRPX0_INPUT_PATH             (19u)
/** \brief macro defining graphics 1 input path */
#define VPS_DC_GRPX1_INPUT_PATH             (20u)
/** \brief macro defining graphics 2 input path */
#define VPS_DC_GRPX2_INPUT_PATH             (21u)

/** \brief macro defining vcomp multiplexer */
#define VPS_DC_VCOMP_MUX                    (1u)
/** \brief macro defining hdcomp multiplexer */
#define VPS_DC_HDCOMP_MUX                   (2u)
/** \brief macro defining sdvenc multiplexer */
#define VPS_DC_SDVENC_MUX                   (3u)


/* Macros for the other VPS modules */

/** \brief Macro defining VCOMP */
#define VPS_DC_VCOMP                          (14u)

/** \brief Macro defining CIG PIP input. In CIG this is the only path
  * which can be disabled. Main input path cannot be disabled. */
#define VPS_DC_CIG_PIP_INPUT                 (16u)

/** \brief macros defining cig constrained output. cig is used for
  * constraning input video. there are three video outputs
  *  from cig, i.e. non-constrained video output (hdmi), which
  *  provides original video stream, constrained
  *  video output (hdcomp), which provides video with the reduced quality
  *  and pip video output, which can be used for pip. */
#define VPS_DC_CIG_CONSTRAINED_OUTPUT          (15u)

/** \brief macros defining cig non-contrained output. */
#define VPS_DC_CIG_NON_CONSTRAINED_OUTPUT      (17u)

/** \brief Macro defining CIG PIP output. This output cannot also be used
  * as pip in the display. Input to the pip is aux path, which can be
  * smaller than the actual display size. In this case, CIG fills rest of the
  * display area with the zero. */
#define VPS_DC_CIG_PIP_OUTPUT                 (18u)


/* macros for the vps blenders */
/** \brief macro for the hdmi blender */
#define VPS_DC_HDMI_BLEND                   (22u)

/** \brief macro for the hdcomp blender */
#define VPS_DC_HDCOMP_BLEND                 (23u)

/** \brief macro for the dvo2 blender */
#define VPS_DC_DVO2_BLEND                   (24u)

/** \brief macro for the sdvenc blender */
#define VPS_DC_SDVENC_BLEND                 (25u)

/** \brief Maximum number of characters in the string for specifying
 *  node name */
#define VPS_DC_MAX_NODE_NAME                (20u)


/**
 *  enum vps_dcnodetype
 *  \brief Enum defining node types available in VPS. There are several
 *  modules available in VPS, which controls how and which display goes
 *  to perticulcar VENC. These path controller modules are Multiplexers,
 *  VCOMP, CIG and Blenders. There modules are known as nodes in the display
 *  controller. This enum defines the type of the node.
 */
enum vps_dcnodetype {
	VPS_DC_NODETYPE_MUX = 0,
	/**< Multiplexer or Switch, takes N selectable inputs and
	     provides one output */
	VPS_DC_NODETYPE_SPLITTER,
	/**< Splitter, takes one input and provides M identical outputs */
	VPS_DC_NODETYPE_COMP,
	/**< Compositor, takes N inputs and provides one composited output */
	VPS_DC_NODETYPE_INPUT,
	/**< Input Node, no node connected as input to this node */
	VPS_DC_NODETYPE_OUTPUT,
	/**< Output Node, no node connected as output from this node*/
	VPS_DC_NODETYPE_MAX
	/**< This must be last enum */
};

/**
 * \brief enum vps_dcvencclkdiv
 *  This enum is used to enable clock divisor for the venc.
 */
enum vps_dcvencclkdiv {
	VPS_DC_VENC_CLK_DIV_HDMI_CLK1X = 0,
	/**< Selects the HD_VENC_D_DVO1 clk1x source clock. It can be
	     either original one or original divided by 2 */
	VPS_DC_VENC_CLK_DIV_HDCOMP_CLK1X,
	/**< HD_VENC_A clk1x source clock. It can be
	     either original one or original divided by 2 */
	VPS_DC_VENC_CLK_DIV_DVO2_CLK1X
	/**< Digital Video Output 2 output clock. It can be
	     either original one or original divided by 2  */
};

/**
 * \brief enum vps_dcvencoutputclk
 *  Enum for selecting source clock for the venc. Output clock from VENC
 *  can be either clk1x or clk2x. */
enum vps_dcvencoutputclk {
	VPS_DC_VENC_CLK_DIV_DVO1,
	/**< Digital Video Output 1 output clock. It could be either  */
	VPS_DC_VENC_CLK_DIV_HDCOMP_VBI,
	/**< VBI HD Clock Select */
	VPS_DC_VENC_CLK_DIV_DVO2
	/**< Digital Video Output 2 Clock 2x Select */
};

/* ======================================================================== */
/*                         Structure Declarations                             */
/* ======================================================================== */


/**
 *	struct vps_dcenumnode
 * \brief Structure containing the properties of a the processing node. Node
 *	represents a VPS module which controls how and which display goes
 *	to perticulcar VENC. This structure is for enumerating this processing
 *	modules one by one and get the properties of it.
 */
struct vps_dcenumnode {
	u32                 nodeidx;
	/**< Index of the node. Node Index starting from 0 to maximum
	     number of nodes. Used at the time of enumerating nodes.
	     After max number of nodes, this function returns zero. */
	u32                 nodeid;
	/* ID of the node*/
	u32                 nodetype;
	/**< Type of the node, This type indicates whether this node can
	accept multiple input or output or not
	for valid values see #vps_dcnodetype*/
	char                nodename[VPS_DC_MAX_NODE_NAME];
	/**< Name of the processing node */
	u32                 numinputs;
	/**< Number of all possible inputs for this node */
	u32                 numoutputs;
	/**< Number of all possible outputs for this node */
};
/**
 *  struct vps_dcenumnodeinput
 * \brief Structure containing the properties of a input of a
 *  the processing node. This structure is for enumerating input of
 *  modules one by one and get the properties of it.
 */
struct vps_dcenumnodeinput {
	u32                 nodeid;
	/**< Id of the node. This is the ID of the node for which input
	     is to be enumerated. */
	u32                 inputidx;
	/**< Index of the input. Input Index starting from 0 to maximum
	     number of inputs of this node. Used at the time of enumerating
	     inputs of the given node. After max number of inputs,
	     this function returns errro. */
	u32                 inputid;
	/**< Input ID of the given node */
	char                inputname[VPS_DC_MAX_NODE_NAME];
	/**< Name of the input */

};

/**
 *  struct Vps_DcEnableNodeInput
 * \brief This structure will be used at the time of enabling/disabling
 *  input of the given node. It takes id of the node and id of the
 *  input for this node.
 */
struct vps_dcnodeinput {
	u32          nodeid;
	/**< Id of the node. Use macros defined above for this id. */
	u32          inputid;
	/**< Input id of the node to be enabled. Use macros defined
	     above for this id. */
	u32          isenable;
	/**< flag to indicate whether given input on the given node
	is enabled or not. 0: disabled, 1: enabled */
};

/**
 *	struct vps_dcvcompconfig
 * \brief structure containing vcomp static configuration
 */
struct vps_dcvcompconfig {
	struct vps_dcyuvcolor        bckgrndcolor;
	/**< background colcor to be displayed in YCrCb */
	struct vps_dcyuvcolor        mainaltcolor;
	/**< alternate colcor to be displayed in YCrCb. When the main source is
	     enabled, send the alternate main Y/Cb/Cr values instead of the
	     true source picture.  This bit allows the datapath to flush
	     thout outputing the actual picture. */
	struct vps_dcyuvcolor        auxaltcolor;
	/**< alternate colcor to be displayed in YCrCb. When the aux source is
	     enabled, send the alternate main Y/Cb/Cr values instead of the
	     true source picture.  This bit allows the datapath to flush
	     through without outputing the actual picture. */
	/* Precedence of the Video */
};



/**
 * struct vps_dcedeconfig
 * structure containing EDE static configuration
 */
struct vps_dcedeconfig {
	u32              ltienable;
	/**< enables/disables luminance transition improvement block */
	u32              horzpeaking;
	/**< enables/disables horizontal peaking */
	u32              ctienable;
	/**< enables/disables chrominance transition improvement block */
	u32              transadjustenable;
	/**< enables/disables transition adjustment for chroma block */
	u32              lumapeaking;
	/**< enables/disables luminance peaking */
	u32              chromapeaking;
	/**< Enables/Disables Chrominance Peaking */
	u16              minclipluma;
	/**< Minimum value of the C clipping in the clipping block */
	u16              maxclipluma;
	/**< Maximum value of the Y clipping in the clipping block */
	u16              minclipchroma;
	/**< Minimum value of the C clipping in the clipping block */
	u16              maxclipchroma;
	/**< Maximum value of the C clipping in the clipping block */
	u32              bypass;
	/**< Bypass complete EDE processing */
};

/**
 * struct vps_dccigmainconfig
 * \brief Structure containing CIG static configuration
 */
struct vps_dccigmainconfig {
	u32                 enablecontraining;
	/**< This enables contraining in the CIG module. Once enabled, it
	     provides controained video on the CIT video output. */
	u32                 nonctrinterlace;
	/**< Enables/Disables interlacing on the non-contraining video output
	     of the CIG. Enabling this will require input video of type
	     progressive and CIG will interlace it and provide interlaced
	     video to the blender */
	u32                ctrinterlace;
	/**< Enables/Disables interlacing on the contraining video output
	     of the CIG. Enabling this will require input video of type
	     progressive and CIG will interlace it and provide interlaced
	     video to the blender */
	u32                vertdecimation;
	/**< Enables the Vertical Decimation. Use only if the contraining is
	     enabled */
};

/**
 * struct Vps_DcCigPipConfig
 * \brief Structure containing static configuration for CIG PIP Path
 */
struct vps_dccigpipconfig {
	u32                 pipinterlace;
	/**< Enables/Disables interlacing on the pip video output
	     of the CIG. Enabling this will require input video of type
	     progressive and CIG will interlace it and provide interlaced video
	     to the blender */
};

/**
 * struct
 * \brief Structure containing COMP static configuration
 */
struct vps_dccompconfig {
	struct vps_dcrgbcolor     bckgrndcolor;
	/**< Background Color in RGB format.his backround color is common for
	     all the compositors/blenders(like HDMI, HDCOMP, DVO2, SD).
	     This color will replace any pixel with RGB value of 000.*/
};


/**
 * struct vps_dcoutputclksel
 * \brief Structure for selecting output clock source for a given venc.
 *  Output clock for a venc can be either 1x clock or 2x clock.
 *  This structure is passed as an argument to the
 *  IOCTL IOCTL_VPS_DCTRL_SEL_OUT_CLK
 */
struct vps_dcoutputclksel {
	u32                 venc;
	/**< Venc for which output clock source to be selected. */
	u32                 isclk2xselect;
	/**< Whether to select clock 2x as an output clock for the given venc.
	    Set TRUE if clk2x is output clock or FALSE if clk1x is the
	    output clock. */
};

/**
 * struct vps_dcvencclkdivsel
 * \brief Structure for selecting clock source for clk1x input. It can
 *  be either original clock i.e. clk2x, or original clock divided by 2.
 *  This structure is used as an argument to the
 *  IOCTL IOCTL_VPS_DCTRL_SEL_CLK1X_SRC
 */
struct vps_dcvencclkdivsel {
	u32                 venc;
	/**< Venc for which clk1x clock source to be selected */
	u32                 isdivide;
	/**< Flag to indicate whether original clock is divided or not. */
};

/**
 * struct vps_dccreateconfig
 * \brief Structure for static configuration
 */
struct vps_dccreateconfig {
	struct vps_dcvcompconfig       *vcompconfig;
	/**< Vcomp Configuration */
	struct vps_dcedeconfig         *edeconfig;
	/**< Ede Configuration */
	struct  vps_dccigmainconfig    *cigmainconfig;
	/**< Cig Configuration for the Main Path */
	struct vps_dccigpipconfig      *cigpipconfig;
	/**< Cig Configuration for the PIP Path */
	/* Dc_CprocConfig cprocConfig; */
	/**< Cproc Configuration */
	struct vps_dccompconfig        *compconfig;
	/**< Comp Configuration */
	struct vps_cscconfig           *sdcscconfig;
	/**< CSC Configuraton for the CSC on SD path */
	struct vps_cscconfig           *hdcompcscconfig;
	/**< CSC Configuraton for the CSC on HDComp */
};

/* ========================================================================== */
/* ========================================================================== */
/*                      part-4 on-chip encoder configuration                  */
/* ========================================================================== */
/* ========================================================================== */

/* ========================================================================== */
/*                           control command                                  */
/* ========================================================================== */

/**
  \addtogroup vps_drv_fvid2_ioctl_display_ctrl
*/

/** \brief on-chip encoder control base
 *
 *  marker used to denote the begining of ioctls that would be required to
 *  configure/control on-chip encoders
 *
 *  \par caution ensure that basic ioctl value does not execeed this value
 */
#define VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE (VPS_DCTRL_IOCTL_ADV_MAX + 0x1u)

/**
 *
 * \brief this control command is used to attach application supplied callback
 *        that would be called on detection / removal of sink.
 *
 * \param   cmdargs [in] pointer of type vps_dconchipencodercmd, which
 *                       initializes following members
 *                       .vencid   = paired venc,vps_dc_venc_hdmi in this case.
 *                       .encoder  = vps_dc_encoder_hdmi
 *                       .cmd      = this macro
 *                       .argument = function of type #fvid2_cbfxn
 *                       .additionalargs = arguments that should be given back
 *                                         when the function is called
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_HDMI_ATTACH_HPD_CB                                  \
				     (VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE + 0x01u)

/**
 *
 * \brief this control command reterive the current hdmi configuration.
 *
 * \param   cmdargs [in] pointer of type vps_dconchipencodercmd, which
 *                       initializes following members
 *                       .vencid   = paired venc,vps_dc_venc_hdmi in this case.
 *                       .encoder  = vps_dc_encoder_hdmi
 *                       .cmd      = this macro
 *                       .argument = pointer to structure of type
 *                                     #vps_hdmiconfigparams
 *                       .additionalargs = null
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_HDMI_GET_CONFIG                                     \
				    (VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE + 0x02u)

/**
 *
 * \brief this control command applies the supplied hdmi configuration.
 *	  provided the values supplied are valid and encoder is not
 *	  enabled.
 *
 * \param   cmdargs [in] pointer of type vps_dconchipencodercmd, which
 *                       initializes following members
 *                       .vencid   = paired venc,vps_dc_venc_hdmi in this case.
 *                       .encoder  = vps_dc_encoder_hdmi
 *                       .cmd      = this macro
 *                       .argument = pointer to structure of type
 *                                     #vps_hdmiedidreadparams
 *                       .additionalargs = null
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_HDMI_SET_CONFIG                                     \
				    (VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE + 0x03u)

/**
 *
 * \brief this control command could be used to get the attached sink edid
 *        information. this command would be honored only if a sink is dected.
 *
 * \param   cmdargs [in] pointer of type vps_dconchipencodercmd, which
 *                       initializes following members
 *                       .vencid    = paired venc,vps_dc_venc_hdmi in this case.
 *                       .encoder   = vps_dc_encoder_hdmi
 *                       .cmd       = this macro
 *                       .argument  = pointer to structure of type
 *                                     #vps_hdmiedidreadparams
 *                       .additionalargs = null
 *
 * \return  vps_sok if successful, else suitable error code
 */
#define IOCTL_VPS_DCTRL_HDMI_READ_EDID                                      \
				    (VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE + 0x04u)


/** \brief maximum number of on-chip encoders control/configure ioctl commands
 *
 *  marker used to denote the maximum number of on-chip ioctls supported
 *
 *  \par caution ensure that on-chip encoders ioctl value does not execeed this
 *       value
 */
#define VPS_DCTRL_IOCTL_ONCHIP_ENC_MAX (VPS_DCTRL_IOCTL_ONCHIP_ENC_BASE + 0x20u)

/* @} */


/* ========================================================================== */
/*             structure required for on-chip encoder control                 */
/* ========================================================================== */

/**
 * \brief
 *  on-chip encoders control parameters. place holder for arguments thats would
 *  be required by the on-chip encoder/its paired venc.
 *
 * \par caution - refer the encoder specific hals header files for the types
 *                of arguments required by the encoders.
 */
struct vps_dconchipencodercmd {
	u32                 vencid;
	/**< venc identifier - venc that would drive encoder of intreset */
	u32                 encoder;
	/**< encoder identifier - selects the encoder that would require
	     perform the requested control */
	u32                 cmd;
	/**< command for the encoder */
	void                *argument;
	/**< command arguments that would be required. */
	void                *additionalargs;
	/**< additional arguments if any. */
};


#endif

#endif /* End of #ifndef _VPS_DISPLAYCTRL_H */

/* @} */
