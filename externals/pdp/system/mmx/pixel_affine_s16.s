#    Pure Data Packet mmx routine.
#    Copyright (c) by Tom Schouten <pdp@zzz.kotnet.org>
# 
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
# 
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
# 
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
.globl pixel_affine_s16
.type  pixel_affine_s16,@function

# void pixel_affine_s16(int *buf, int nb_8pixel_vectors, short int gain[4], short int offset[4])

pixel_affine_s16:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

	movl 20(%ebp), %edi
	movq (%edi), %mm6	# get offset vector

	movl 16(%ebp), %edi
	movq (%edi), %mm7	# get gain vector
	
	movl 8(%ebp),  %esi	# input array
	movl 12(%ebp), %ecx	# pixel count

	
	.align 16
	.loop_affine:	

#	prefetch 128(%esi)	
	movq (%esi), %mm0	# load 4 pixels from memory
	pmulhw %mm7, %mm0	# apply gain (s).15 fixed point
	psllw $1, %mm0		# apply correction shift
	paddsw %mm6, %mm0	# add offset
	movq %mm0, (%esi)	# store result in memory

	addl $8, %esi		# increment source pointer
	decl %ecx
	jnz .loop_affine	# loop

	emms
	
	pop %edi
	pop %esi
	leave
	ret
	
