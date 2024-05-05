/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020-2021 Loongson Technology Corporation Limited
 */

#include <asm/asmmacro.h>
#include <asm/export.h>
#include <asm/regdef.h>
#include <asm/asm.h>
/*
 * void *memcpy(void *dst, const void *src, size_t n)
 *
 * a0: dst
 * a1: src
 * a2: n
 */
SYM_FUNC_START(memcpy)
	beqz	a2, 4f     // 直接返回
	move	a3, a0     // 保存返回值

	or      t0, a0, a1 // 检查对齐
	slti    t1, a2, 8  // 检查 n < 8

	andi    t0, t0, 7    // 8 对齐
	ori     t2, zero, 8  // 加载循环退出条件

	or      t0, t0, t1   // 不对齐或者 n < 8

	bnez    t0, 2f       // 不符合检查条件，进慢循环
// 8Bytes align, Fast copy
1:	ld.w	t0, a1, 0
	ld.w	t1, a1, 4    // 加载 8 Bytes

	LONG_ADDIU	a1, a1, 8  // 更新 src += 8bytes
	LONG_ADDIU	a2, a2, -8 // 更新 n -= 8

	st.w	t0, a0, 0
	st.w	t1, a0, 4    // 存 8 Bytes

	LONG_ADDIU	a0, a0, 8
	bge     a2, t2, 1b // n >= 8
	beq     a2, zero, 3f

2:	ld.b	t0, a1, 0
	LONG_ADDIU	a1, a1, 1
	LONG_ADDIU	a2, a2, -1
	st.b	t0, a0, 0
	LONG_ADDIU	a0, a0, 1
	bgt	a2, zero, 2b

3:  move	v0, a3
4:  jr	ra
SYM_FUNC_END(memcpy)

EXPORT_SYMBOL(memcpy)
