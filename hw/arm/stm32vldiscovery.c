/*
 * ST STM32VLDISCOVERY machine
 *
 * Copyright (c) 2021 Alexandre Iooss <erdnaxe@crans.org>
 * Copyright (c) 2014 Alistair Francis <alistair@alistair23.me>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/qdev-properties.h"
#include "hw/qdev-clock.h"
#include "qemu/error-report.h"
#include "hw/arm/stm32f100_soc.h"
#include "hw/arm/boot.h"

/* stm32vldiscovery implementation is derived from netduinoplus2 */
#define TYPE_STM32VLDISCOVERY "stm32vldiscovery"
OBJECT_DECLARE_SIMPLE_TYPE(STM32VLDISCOVERYState, STM32VLDISCOVERY)

typedef struct STM32VLDISCOVERYState {
    MachineState parent_obj;
    STM32F100State soc;
} STM32VLDISCOVERYState;
/* Main SYSCLK frequency in Hz (24MHz) */
#define SYSCLK_FRQ 24000000ULL

static void stm32vldiscovery_init(MachineState *machine)
{
    STM32VLDISCOVERYState *s = STM32VLDISCOVERY(machine);
    DeviceState *dev = DEVICE(&s->soc);
    Clock *sysclk;

    /* This clock doesn't need migration because it is fixed-frequency */
    sysclk = clock_new(OBJECT(machine), "SYSCLK");
    clock_set_hz(sysclk, SYSCLK_FRQ);

    object_initialize_child(OBJECT(machine), "soc", &s->soc, TYPE_STM32F100_SOC);
    qdev_connect_clock_in(dev, "sysclk", sysclk);
    sysbus_realize(SYS_BUS_DEVICE(&s->soc), &error_fatal);

    armv7m_load_kernel(s->soc.armv7m.cpu,
                       machine->kernel_filename,
                       0, FLASH_SIZE);
}

static void stm32vldiscovery_machine_init(ObjectClass *oc, const void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    static const char * const valid_cpu_types[] = {
        ARM_CPU_TYPE_NAME("cortex-m3"),
        NULL
    };

    mc->desc = "ST STM32VLDISCOVERY (Cortex-M3)";
    mc->init = stm32vldiscovery_init;
    mc->valid_cpu_types = valid_cpu_types;
}

static const TypeInfo stm32vldiscovery_machine_type[] = {
    {
        .name           = TYPE_STM32VLDISCOVERY,
        .parent         = TYPE_MACHINE,
        .instance_size  = sizeof(STM32VLDISCOVERYState),
        .class_init     = stm32vldiscovery_machine_init,
    }
};


DEFINE_TYPES(stm32vldiscovery_machine_type)
