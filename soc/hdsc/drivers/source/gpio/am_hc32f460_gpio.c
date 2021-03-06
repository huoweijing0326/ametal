/*******************************************************************************
*                                 AMetal
*                       ----------------------------
*                       innovating embedded platform
*
* Copyright (c) 2001-2018 Guangzhou ZHIYUAN Electronics Co., Ltd.
* All rights reserved.
*
* Contact information:
* web site:    http://www.zlg.cn/
*******************************************************************************/

/**
 * \file
 * \brief GPIO驱动，服务GPIO标准接口
 *
 * \internal
 * \par Modification history
 * - 1.00 19-02-22  ipk,first implementation
 * \endinternal
 */

#include <am_hc32f460_gpio.h>
#include <am_hc32f460_gpio_util.h>
#include "am_gpio_util.h"
#include "am_gpio.h"
#include "am_int.h"
#include "am_bitops.h"
//#include "hw/amhw_zmf159_exti.h"
//#include "zmf159_pin.h"

/*******************************************************************************
* 私有定义
*******************************************************************************/

/** \brief 中断未连接标识 */
#define AM_HC32F460_GPIO_INVALID_PIN_MAP    0xFF

/** \brief 定义指向GPIO设备信息的指针 */
#define __GPIO_DEVINFO_DECL(p_gpio_devinfo, p_dev)  \
        const am_hc32f460_gpio_devinfo_t *p_gpio_devinfo = p_dev->p_devinfo

/******************************************************************************
  全局变量
******************************************************************************/

/** \bruef 指向GPIO设备的指针 */
am_hc32f460_gpio_dev_t *__gp_gpio_dev;

/**
 * \brief 外部中断线0中断函数
 */
static void __port_exit0_int_isr (void * p_arg)
{

}

/**
 * \brief 外部中断线1中断函数
 */
static void __port_exit1_int_isr (void * p_arg)
{

}

/**
 * \brief 外部中断线2中断函数
 */
static void __port_exit2_int_isr (void * p_arg)
{

}

/**
 * \brief 外部中断线2中断函数
 */
static void __port_exit3_int_isr (void * p_arg)
{

}

/**
 * \brief 外部中断线4中断函数
 */
static void __port_exit4_int_isr (void * p_arg)
{

}

/**
 * \brief 外部中断线9_5中断函数
 */
static void __port_exit9_5_int_isr (void * p_arg)
{

}

/**
 * \brief 外部中断线5_10中断函数
 */
static void __port_exit15_10_int_isr (void * p_arg)
{

}

/*******************************************************************************
  公共函数
*******************************************************************************/

/**
 * \brief 引脚功能配置
 *
 * \param[in] pin       : 引脚编号，值为 PIO* (#PIOA_0)
 * \param[in] flags     : 引脚功能
 *
 * \return AM_OK     ：配置成功
 */
int am_gpio_pin_cfg (int pin, uint32_t flags)
{
    __GPIO_DEVINFO_DECL(p_gpio_devinfo, __gp_gpio_dev);

    /* IO口方向 */
    uint8_t dir = 0;
    uint32_t func = 0, mode = 0;
    amhw_hc32f460_gpio_t *p_hw_gpio  = NULL;
    amhw_hc32f460_gpio_mode_t pin_mode   = AMHW_HC32F460_GPIO_MODE_IPU;

    uint8_t pin_pos = 0u;
    en_portx_t en_portx = AMHW_HC32F460_GPIO_PORT_A;

    if (__gp_gpio_dev == NULL) {
        return -AM_ENXIO;
    }
    
    __pin_calc(pin, &en_portx, &pin_pos);
    p_hw_gpio = (amhw_hc32f460_gpio_t*)p_gpio_devinfo->gpio_regbase;

    func      = AM_GPIO_COM_FUNC_GET(flags);
    mode      = AM_GPIO_COM_MODE_GET(flags);

    /* 标准层相关 */
    if (func != AM_GPIO_FUNC_INVALID_VAL) {
        /* 使用标准层前先退回引脚为GPIO模式 */
        if (amhw_hc32f460_gpio_pin_afr_get(p_hw_gpio, pin) != AMHW_HC32F460_AFIO_GPIO) {

            /* 复用功能回到默认状态，表明使用GPIO功能 */
            amhw_hc32f460_gpio_pin_afr_set(p_hw_gpio, AMHW_HC32F460_AFIO_GPIO, pin);

        }

        switch (func) {

        case AM_GPIO_INPUT_VAL:
            amhw_hc32f460_gpio_pin_dir_input(p_hw_gpio, pin);
            break;

        case AM_GPIO_OUTPUT_INIT_HIGH_VAL:
            /* 推挽输出 */
            amhw_hc32f460_gpio_pin_mode_set(p_hw_gpio,
                                          AMHW_HC32F460_GPIO_MODE_OUT_PP,
                                          pin);
            /* 输出高电平 */                                          
            amhw_hc32f460_gpio_pin_out_high(p_hw_gpio, pin);
            break;

        case AM_GPIO_OUTPUT_INIT_LOW_VAL:
            /* 推挽输出 */
            amhw_hc32f460_gpio_pin_mode_set(p_hw_gpio,
                                          AMHW_HC32F460_GPIO_MODE_OUT_PP,
                                          pin);

            /* 输出低电平 */
            amhw_hc32f460_gpio_pin_out_low(p_hw_gpio, pin);
            break;

        default:
            return -AM_ENOTSUP;
        }
    }
    
    /* 获取输入输出方向 */
    
    if (mode != AM_GPIO_MODE_INVALID_VAL) {
        dir = amhw_hc32f460_gpio_pin_dir_get(p_hw_gpio, pin);

        switch (mode) {              
        case AM_GPIO_PULL_UP_VAL:
            if (dir != 1) {
                /* 上拉输入模式  */
                pin_mode = AMHW_HC32F460_GPIO_MODE_IPU;
            } else {
                /* 输出没有上拉模式，设置为推挽模式 */
                pin_mode = AMHW_HC32F460_GPIO_MODE_OUT_PP;
            }
            break;

        case AM_GPIO_PULL_DOWN_VAL:  //HC32F46X cannot Support 
            break;

        case AM_GPIO_FLOAT_VAL:    //HC32F46X cannot Support 
            if (dir != 1) {
                pin_mode = AMHW_HC32F460_GPIO_MODE_IN_FLOATING;
            } else {
                /* 输出没有浮空模式，设置为开漏模式 */
                pin_mode = AMHW_HC32F460_GPIO_MODE_OUT_OD;
            }
            break;

        case AM_GPIO_OPEN_DRAIN_VAL:
            if (dir != 1) {
                /* 输入没有开漏模式，设置为浮空 */
                pin_mode = AMHW_HC32F460_GPIO_MODE_IN_FLOATING;
            } else {
                pin_mode = AMHW_HC32F460_GPIO_MODE_OUT_OD;
            }

            break;

        case AM_GPIO_PUSH_PULL_VAL:
            if (dir != 1) {
                /* 输入没有推挽模式，设置为上拉 */
                pin_mode = AMHW_HC32F460_GPIO_MODE_IPU;
            } else {
                pin_mode = AMHW_HC32F460_GPIO_MODE_OUT_PP;
            }
            break;

        default:
            return -AM_ENOTSUP;
        }        
        amhw_hc32f460_gpio_pin_mode_set(p_hw_gpio, pin_mode, pin);
    }

    /*
     * 平台相关配置
     */

    /* 管脚的模式 */
    if ((flags & AM_HC32F460_GPIO_MODE) != 0) {

       switch (AM_ZMF159_GPIO_MODE_GET(flags)) {

       case 0:
           /* 上拉输入 */
           pin_mode = AMHW_HC32F460_GPIO_MODE_AIN;
           break;

       case 1:
           /* 浮空输入 */       
           pin_mode = AMHW_HC32F460_GPIO_MODE_IN_FLOATING;       
           break;

       case 2:  
           /* 下拉输入模式  */
           pin_mode = AMHW_HC32F460_GPIO_MODE_IPD;                             
           break;

       case 3:
           /* 上拉输入模式  */
           pin_mode = AMHW_HC32F460_GPIO_MODE_IPU;                  
           break;

       case 4:
           /* 通用推挽输出模式 */
           pin_mode = AMHW_HC32F460_GPIO_MODE_OUT_PP;                  
           break;

       case 5:
           /* 开漏输出模式 */
           pin_mode = AMHW_HC32F460_GPIO_MODE_OUT_OD;
           break;

       case 6:   
           /* 复用推挽输出模式 */
           pin_mode = AMHW_HC32F460_GPIO_MODE_AOUT;
           break;

       case 7:  
           /* 复用开漏输出模式 */
           pin_mode = AMHW_HC32F460_GPIO_MODE_AF_PP;
           break;
           
       case 8:  
           /* 复用开漏输出模式 */
           pin_mode = AMHW_HC32F460_GPIO_MODE_AF_OD;
           break;

       default:
           return -AM_ENOTSUP;
       }       
       amhw_hc32f460_gpio_pin_mode_set(p_hw_gpio, pin_mode, pin);
    }

    /* 设置管脚的输出速率 */
    if ((flags & AM_HC32F460_GPIO_OUTRES_RATE) != 0) {
        amhw_hc32f460_gpio_pin_driver_capability(p_hw_gpio, AM_ZMF159_GPIO_OUTRES_RATE_GET(flags), pin);
    }

    /* 设置管脚的复用功能 */
    if ((flags & AM_HC32F460_GPIO_FUNEN) != 0) {
        amhw_hc32f460_gpio_pin_afr_set(p_hw_gpio, AM_ZMF159_GPIO_FUNC_GET(flags), pin);
    }
    return AM_OK;
}

/**
 * \brief 获取引脚状态
 *
 * \param[in] pin       : 引脚编号，值为 PIO* (#PIOA_0)
 *
 * \return 无
 */
int am_gpio_get (int pin)
{
    __GPIO_DEVINFO_DECL(p_gpio_devinfo, __gp_gpio_dev);

    amhw_hc32f460_gpio_t *p_hw_gpio = (amhw_hc32f460_gpio_t *)p_gpio_devinfo->gpio_regbase;

    return amhw_hc32f460_gpio_pin_get(p_hw_gpio, pin);
}

/**
 * \brief 设置输出引脚状态
 *
 * \param[in] pin       : 引脚编号，值为 PIO* (#PIOA_0)
 * \param[in] value     : 引脚值
 *
 * \return AM_OK     ：设置成功
 */
int am_gpio_set (int pin, int value)
{
    __GPIO_DEVINFO_DECL(p_gpio_devinfo, __gp_gpio_dev);

    amhw_hc32f460_gpio_t *p_hw_gpio = (amhw_hc32f460_gpio_t *)p_gpio_devinfo->gpio_regbase;

    if (value == 0) {
        amhw_hc32f460_gpio_pin_out_low(p_hw_gpio, pin);
    } else {
        amhw_hc32f460_gpio_pin_out_high(p_hw_gpio, pin);

    }
    return AM_OK;
}

/**
 * \brief 引脚输出状态反转
 */
int am_gpio_toggle (int pin)
{
    __GPIO_DEVINFO_DECL(p_gpio_devinfo, __gp_gpio_dev);

    amhw_hc32f460_gpio_t *p_hw_gpio = (amhw_hc32f460_gpio_t *)p_gpio_devinfo->gpio_regbase;

    amhw_hc32f460_gpio_pin_toggle(p_hw_gpio, pin);

    return AM_OK;
}

/**
 * \brief 引脚触发形式设置
 */
int am_gpio_trigger_cfg (int pin, uint32_t flag)
{
    return AM_OK;
}

/**
 * \brief 使能引脚中断。
 */
int am_gpio_trigger_on (int pin)
{
    return AM_OK;
}

/**
 * \brief 禁能引脚中断。
 */
int am_gpio_trigger_off (int pin)
{
    return AM_OK;
}

/**
 * \brief 连接引脚中断回调函数
 */
int am_gpio_trigger_connect (int           pin,
                             am_pfnvoid_t  pfn_callback,
                             void         *p_arg)
{
     return AM_OK;
}

/**
 * \brief 删除引脚中断回调函数连接
 */
int am_gpio_trigger_disconnect (int           pin,
                                am_pfnvoid_t  pfn_callback,
                                void         *p_arg)
{
    return AM_OK;
}

/**
 * \brief GPIO初始化
 *
 * \param[in] p_dev     : 指向GPIO设备的指针
 * \param[in] p_devinfo : 指向GPIO设备信息的指针
 *
 * \retval AM_OK : 操作成功
 */
int am_hc32f460_gpio_init (am_hc32f460_gpio_dev_t           *p_dev,
                           const am_hc32f460_gpio_devinfo_t *p_devinfo)
{
    uint8_t i = 0;

    if (NULL == p_dev || NULL == p_devinfo) {
        return -AM_EINVAL;
    }

    if (p_devinfo->pfn_plfm_init) {
        p_devinfo->pfn_plfm_init();
    }

    p_dev->p_devinfo = p_devinfo;

    if ((p_devinfo->inum_pin == NULL)  ||
        (p_devinfo->p_infomap == NULL) ||
        (p_devinfo->p_triginfo == NULL)) {
        p_dev->valid_flg = AM_FALSE;
        return -AM_EINVAL;
    }

//    for (i = 0 ; i < p_devinfo->pin_count ; i++) {
//        if ((i == PIOA_13) || (i == PIOA_14) || (i == PIOA_15) ||
//             (i == PIOB_3) || (i == PIOB_4)) {
//            p_devinfo->p_remap[i] = AMHW_ZLG217_SWJ_CFG;
//        } else {
//            p_devinfo->p_remap[i] = AMHW_ZLG217_NO_REMAP;
//        }
//    }

    for (i = 0 ; i < p_devinfo->exti_num_max ; i++) {
        p_devinfo->p_infomap[i] = AM_HC32F460_GPIO_INVALID_PIN_MAP;
        p_devinfo->p_triginfo[i].p_arg = NULL;
        p_devinfo->p_triginfo[i].pfn_callback = NULL;
    }

//    am_int_connect(p_devinfo->inum_pin[0], __port_exit0_int_isr, NULL);
//    am_int_connect(p_devinfo->inum_pin[1], __port_exit1_int_isr, NULL);
//    am_int_connect(p_devinfo->inum_pin[2], __port_exit2_int_isr, NULL);
//    am_int_connect(p_devinfo->inum_pin[3], __port_exit3_int_isr, NULL);
//    am_int_connect(p_devinfo->inum_pin[4], __port_exit4_int_isr, NULL);

//    am_int_connect(p_devinfo->inum_pin[5], __port_exit9_5_int_isr, NULL);
//    am_int_connect(p_devinfo->inum_pin[6], __port_exit15_10_int_isr, NULL);

//    am_int_enable(p_devinfo->inum_pin[0]);
//    am_int_enable(p_devinfo->inum_pin[1]);
//    am_int_enable(p_devinfo->inum_pin[2]);
//    am_int_enable(p_devinfo->inum_pin[3]);
//    am_int_enable(p_devinfo->inum_pin[4]);
//    am_int_enable(p_devinfo->inum_pin[5]);
//    am_int_enable(p_devinfo->inum_pin[6]);

    p_dev->valid_flg = AM_TRUE;

    __gp_gpio_dev = p_dev;

    return AM_OK;
}

/**
 * \brief GPIO去初始化
 *
 * \param[in] 无
 *
 * \return 无
 */
void am_hc32f460_gpio_deinit (void)
{
    __GPIO_DEVINFO_DECL(p_gpio_devinfo, __gp_gpio_dev);

    int i = 0;

    if (__gp_gpio_dev->valid_flg) {
        for (i = 0; i < p_gpio_devinfo->exti_num_max; i++) {
            (p_gpio_devinfo->p_infomap)[i]               = AM_HC32F460_GPIO_INVALID_PIN_MAP;
            (p_gpio_devinfo->p_triginfo)[i].pfn_callback = NULL;
        }
    }

//    am_int_disable((p_gpio_devinfo->inum_pin)[0]);
//    am_int_disable((p_gpio_devinfo->inum_pin)[1]);
//    am_int_disable((p_gpio_devinfo->inum_pin)[2]);

//    am_int_disconnect(p_gpio_devinfo->inum_pin[0], __port_exit0_int_isr, NULL);
//    am_int_disconnect(p_gpio_devinfo->inum_pin[1], __port_exit1_int_isr, NULL);
//    am_int_disconnect(p_gpio_devinfo->inum_pin[2], __port_exit2_int_isr, NULL);
//    am_int_disconnect(p_gpio_devinfo->inum_pin[3], __port_exit3_int_isr, NULL);
//    am_int_disconnect(p_gpio_devinfo->inum_pin[4], __port_exit4_int_isr, NULL);

//    am_int_disconnect(p_gpio_devinfo->inum_pin[5], __port_exit9_5_int_isr, NULL);
//    am_int_disconnect(p_gpio_devinfo->inum_pin[6], __port_exit15_10_int_isr, NULL);

    if (__gp_gpio_dev->p_devinfo->pfn_plfm_deinit) {
        __gp_gpio_dev->p_devinfo->pfn_plfm_deinit();
    }
}

/* end of file */
