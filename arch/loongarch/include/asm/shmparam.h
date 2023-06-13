/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020-2021 Loongson Technology Corporation Limited
 */
#ifndef _ASM_SHMPARAM_H
#define _ASM_SHMPARAM_H

#define __ARCH_FORCE_SHMLBA	1

#ifdef CONFIG_32BIT
#define	SHMLBA	SZ_16K		 /* attach addr a multiple of this */
#else
#define	SHMLBA	SZ_64K		 /* attach addr a multiple of this */
#endif

#endif /* _ASM_SHMPARAM_H */
