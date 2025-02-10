
Example.elf:     file format elf32-littlearm


Disassembly of section .text:

00080000 <exception_table>:
   80000:	b0 08 00 20 69 01 08 00 65 01 08 00 65 01 08 00     ... i...e...e...
   80010:	65 01 08 00 65 01 08 00 65 01 08 00 00 00 00 00     e...e...e.......
	...
   8002c:	65 01 08 00 65 01 08 00 00 00 00 00 65 01 08 00     e...e.......e...
   8003c:	65 01 08 00 65 01 08 00 65 01 08 00 65 01 08 00     e...e...e...e...
   8004c:	65 01 08 00 65 01 08 00 65 01 08 00 65 01 08 00     e...e...e...e...
   8005c:	65 01 08 00 65 01 08 00 00 00 00 00 00 00 00 00     e...e...........
   8006c:	65 01 08 00 65 01 08 00 00 00 00 00 00 00 00 00     e...e...........
	...
   80084:	65 01 08 00 65 01 08 00 65 01 08 00 00 00 00 00     e...e...e.......
   80094:	65 01 08 00 65 01 08 00 65 01 08 00 65 01 08 00     e...e...e...e...
   800a4:	00 00 00 00 65 01 08 00 65 01 08 00 65 01 08 00     ....e...e...e...
   800b4:	65 01 08 00 65 01 08 00 65 01 08 00 65 01 08 00     e...e...e...e...
	...
   800d0:	65 01 08 00 65 01 08 00 65 01 08 00 65 01 08 00     e...e...e...e...
   800e0:	65 01 08 00 65 01 08 00 00 00 00 00 65 01 08 00     e...e.......e...
   800f0:	65 01 08 00                                         e...

000800f4 <__do_global_dtors_aux>:
   800f4:	b510      	push	{r4, lr}
   800f6:	4c07      	ldr	r4, [pc, #28]	; (80114 <__do_global_dtors_aux+0x20>)
   800f8:	7823      	ldrb	r3, [r4, #0]
   800fa:	2b00      	cmp	r3, #0
   800fc:	d107      	bne.n	8010e <__do_global_dtors_aux+0x1a>
   800fe:	4b06      	ldr	r3, [pc, #24]	; (80118 <__do_global_dtors_aux+0x24>)
   80100:	2b00      	cmp	r3, #0
   80102:	d002      	beq.n	8010a <__do_global_dtors_aux+0x16>
   80104:	4805      	ldr	r0, [pc, #20]	; (8011c <__do_global_dtors_aux+0x28>)
   80106:	e000      	b.n	8010a <__do_global_dtors_aux+0x16>
   80108:	bf00      	nop
   8010a:	2301      	movs	r3, #1
   8010c:	7023      	strb	r3, [r4, #0]
   8010e:	bc10      	pop	{r4}
   80110:	bc01      	pop	{r0}
   80112:	4700      	bx	r0
   80114:	20000430 	.word	0x20000430
   80118:	00000000 	.word	0x00000000
   8011c:	000803e0 	.word	0x000803e0

00080120 <frame_dummy>:
   80120:	4b0a      	ldr	r3, [pc, #40]	; (8014c <frame_dummy+0x2c>)
   80122:	b510      	push	{r4, lr}
   80124:	2b00      	cmp	r3, #0
   80126:	d003      	beq.n	80130 <frame_dummy+0x10>
   80128:	4909      	ldr	r1, [pc, #36]	; (80150 <frame_dummy+0x30>)
   8012a:	480a      	ldr	r0, [pc, #40]	; (80154 <frame_dummy+0x34>)
   8012c:	e000      	b.n	80130 <frame_dummy+0x10>
   8012e:	bf00      	nop
   80130:	4809      	ldr	r0, [pc, #36]	; (80158 <frame_dummy+0x38>)
   80132:	6803      	ldr	r3, [r0, #0]
   80134:	2b00      	cmp	r3, #0
   80136:	d102      	bne.n	8013e <frame_dummy+0x1e>
   80138:	bc10      	pop	{r4}
   8013a:	bc01      	pop	{r0}
   8013c:	4700      	bx	r0
   8013e:	4b07      	ldr	r3, [pc, #28]	; (8015c <frame_dummy+0x3c>)
   80140:	2b00      	cmp	r3, #0
   80142:	d0f9      	beq.n	80138 <frame_dummy+0x18>
   80144:	f000 f80c 	bl	80160 <frame_dummy+0x40>
   80148:	e7f6      	b.n	80138 <frame_dummy+0x18>
   8014a:	46c0      	nop			; (mov r8, r8)
   8014c:	00000000 	.word	0x00000000
   80150:	20000434 	.word	0x20000434
   80154:	000803e0 	.word	0x000803e0
   80158:	000803e0 	.word	0x000803e0
   8015c:	00000000 	.word	0x00000000
   80160:	4718      	bx	r3
   80162:	46c0      	nop			; (mov r8, r8)

00080164 <Dummy_Handler>:
   80164:	e7fe      	b.n	80164 <Dummy_Handler>
	...

00080168 <Reset_Handler>:
   80168:	b510      	push	{r4, lr}
   8016a:	4a0e      	ldr	r2, [pc, #56]	; (801a4 <Reset_Handler+0x3c>)
   8016c:	4b0e      	ldr	r3, [pc, #56]	; (801a8 <Reset_Handler+0x40>)
   8016e:	429a      	cmp	r2, r3
   80170:	d009      	beq.n	80186 <Reset_Handler+0x1e>
   80172:	4b0d      	ldr	r3, [pc, #52]	; (801a8 <Reset_Handler+0x40>)
   80174:	4a0b      	ldr	r2, [pc, #44]	; (801a4 <Reset_Handler+0x3c>)
   80176:	e003      	b.n	80180 <Reset_Handler+0x18>
   80178:	6811      	ldr	r1, [r2, #0]
   8017a:	6019      	str	r1, [r3, #0]
   8017c:	3304      	adds	r3, #4
   8017e:	3204      	adds	r2, #4
   80180:	490a      	ldr	r1, [pc, #40]	; (801ac <Reset_Handler+0x44>)
   80182:	428b      	cmp	r3, r1
   80184:	d3f8      	bcc.n	80178 <Reset_Handler+0x10>
   80186:	4b0a      	ldr	r3, [pc, #40]	; (801b0 <Reset_Handler+0x48>)
   80188:	e002      	b.n	80190 <Reset_Handler+0x28>
   8018a:	2200      	movs	r2, #0
   8018c:	601a      	str	r2, [r3, #0]
   8018e:	3304      	adds	r3, #4
   80190:	4a08      	ldr	r2, [pc, #32]	; (801b4 <Reset_Handler+0x4c>)
   80192:	4293      	cmp	r3, r2
   80194:	d3f9      	bcc.n	8018a <Reset_Handler+0x22>
   80196:	4b08      	ldr	r3, [pc, #32]	; (801b8 <Reset_Handler+0x50>)
   80198:	f000 f812 	bl	801c0 <Reset_Handler+0x58>
   8019c:	4b07      	ldr	r3, [pc, #28]	; (801bc <Reset_Handler+0x54>)
   8019e:	f000 f80f 	bl	801c0 <Reset_Handler+0x58>
   801a2:	e7fe      	b.n	801a2 <Reset_Handler+0x3a>
   801a4:	000803e0 	.word	0x000803e0
   801a8:	20000000 	.word	0x20000000
   801ac:	2000042c 	.word	0x2000042c
   801b0:	20000430 	.word	0x20000430
   801b4:	200004b0 	.word	0x200004b0
   801b8:	000801f5 	.word	0x000801f5
   801bc:	000801c5 	.word	0x000801c5
   801c0:	4718      	bx	r3
   801c2:	46c0      	nop			; (mov r8, r8)

000801c4 <main>:
   801c4:	e00c      	b.n	801e0 <main+0x1c>
   801c6:	4908      	ldr	r1, [pc, #32]	; (801e8 <main+0x24>)
   801c8:	680a      	ldr	r2, [r1, #0]
   801ca:	3201      	adds	r2, #1
   801cc:	600a      	str	r2, [r1, #0]
   801ce:	3301      	adds	r3, #1
   801d0:	4a06      	ldr	r2, [pc, #24]	; (801ec <main+0x28>)
   801d2:	6013      	str	r3, [r2, #0]
   801d4:	4b05      	ldr	r3, [pc, #20]	; (801ec <main+0x28>)
   801d6:	681b      	ldr	r3, [r3, #0]
   801d8:	4a05      	ldr	r2, [pc, #20]	; (801f0 <main+0x2c>)
   801da:	4293      	cmp	r3, r2
   801dc:	ddf3      	ble.n	801c6 <main+0x2>
   801de:	46c0      	nop			; (mov r8, r8)
   801e0:	2200      	movs	r2, #0
   801e2:	4b02      	ldr	r3, [pc, #8]	; (801ec <main+0x28>)
   801e4:	601a      	str	r2, [r3, #0]
   801e6:	e7f5      	b.n	801d4 <main+0x10>
   801e8:	20000450 	.word	0x20000450
   801ec:	20000488 	.word	0x20000488
   801f0:	0098967f 	.word	0x0098967f

000801f4 <__libc_init_array>:
   801f4:	b570      	push	{r4, r5, r6, lr}
   801f6:	4e10      	ldr	r6, [pc, #64]	; (80238 <__libc_init_array+0x44>)
   801f8:	4d10      	ldr	r5, [pc, #64]	; (8023c <__libc_init_array+0x48>)
   801fa:	1bad      	subs	r5, r5, r6
   801fc:	10ad      	asrs	r5, r5, #2
   801fe:	2400      	movs	r4, #0
   80200:	2d00      	cmp	r5, #0
   80202:	d006      	beq.n	80212 <__libc_init_array+0x1e>
   80204:	00a3      	lsls	r3, r4, #2
   80206:	58f3      	ldr	r3, [r6, r3]
   80208:	3401      	adds	r4, #1
   8020a:	f000 f81d 	bl	80248 <__libc_init_array+0x54>
   8020e:	42a5      	cmp	r5, r4
   80210:	d1f8      	bne.n	80204 <__libc_init_array+0x10>
   80212:	f000 f8d3 	bl	803bc <_init>
   80216:	4e0a      	ldr	r6, [pc, #40]	; (80240 <__libc_init_array+0x4c>)
   80218:	4d0a      	ldr	r5, [pc, #40]	; (80244 <__libc_init_array+0x50>)
   8021a:	1bad      	subs	r5, r5, r6
   8021c:	10ad      	asrs	r5, r5, #2
   8021e:	2400      	movs	r4, #0
   80220:	2d00      	cmp	r5, #0
   80222:	d006      	beq.n	80232 <__libc_init_array+0x3e>
   80224:	00a3      	lsls	r3, r4, #2
   80226:	58f3      	ldr	r3, [r6, r3]
   80228:	3401      	adds	r4, #1
   8022a:	f000 f80d 	bl	80248 <__libc_init_array+0x54>
   8022e:	42a5      	cmp	r5, r4
   80230:	d1f8      	bne.n	80224 <__libc_init_array+0x30>
   80232:	bc70      	pop	{r4, r5, r6}
   80234:	bc01      	pop	{r0}
   80236:	4700      	bx	r0
   80238:	000803c8 	.word	0x000803c8
   8023c:	000803c8 	.word	0x000803c8
   80240:	000803c8 	.word	0x000803c8
   80244:	000803d0 	.word	0x000803d0
   80248:	4718      	bx	r3
   8024a:	46c0      	nop			; (mov r8, r8)

0008024c <register_fini>:
   8024c:	4b04      	ldr	r3, [pc, #16]	; (80260 <register_fini+0x14>)
   8024e:	b510      	push	{r4, lr}
   80250:	2b00      	cmp	r3, #0
   80252:	d002      	beq.n	8025a <register_fini+0xe>
   80254:	4803      	ldr	r0, [pc, #12]	; (80264 <register_fini+0x18>)
   80256:	f000 f807 	bl	80268 <atexit>
   8025a:	bc10      	pop	{r4}
   8025c:	bc01      	pop	{r0}
   8025e:	4700      	bx	r0
   80260:	00000000 	.word	0x00000000
   80264:	0008027d 	.word	0x0008027d

00080268 <atexit>:
   80268:	b510      	push	{r4, lr}
   8026a:	0001      	movs	r1, r0
   8026c:	2300      	movs	r3, #0
   8026e:	2200      	movs	r2, #0
   80270:	2000      	movs	r0, #0
   80272:	f000 f825 	bl	802c0 <__register_exitproc>
   80276:	bc10      	pop	{r4}
   80278:	bc02      	pop	{r1}
   8027a:	4708      	bx	r1

0008027c <__libc_fini_array>:
   8027c:	b570      	push	{r4, r5, r6, lr}
   8027e:	4b0a      	ldr	r3, [pc, #40]	; (802a8 <__libc_fini_array+0x2c>)
   80280:	4c0a      	ldr	r4, [pc, #40]	; (802ac <__libc_fini_array+0x30>)
   80282:	1ae4      	subs	r4, r4, r3
   80284:	10a4      	asrs	r4, r4, #2
   80286:	d00a      	beq.n	8029e <__libc_fini_array+0x22>
   80288:	4a09      	ldr	r2, [pc, #36]	; (802b0 <__libc_fini_array+0x34>)
   8028a:	18a5      	adds	r5, r4, r2
   8028c:	00ad      	lsls	r5, r5, #2
   8028e:	18ed      	adds	r5, r5, r3
   80290:	682b      	ldr	r3, [r5, #0]
   80292:	3c01      	subs	r4, #1
   80294:	f000 f80e 	bl	802b4 <__libc_fini_array+0x38>
   80298:	3d04      	subs	r5, #4
   8029a:	2c00      	cmp	r4, #0
   8029c:	d1f8      	bne.n	80290 <__libc_fini_array+0x14>
   8029e:	f000 f897 	bl	803d0 <_fini>
   802a2:	bc70      	pop	{r4, r5, r6}
   802a4:	bc01      	pop	{r0}
   802a6:	4700      	bx	r0
   802a8:	000803dc 	.word	0x000803dc
   802ac:	000803e0 	.word	0x000803e0
   802b0:	3fffffff 	.word	0x3fffffff
   802b4:	4718      	bx	r3
   802b6:	46c0      	nop			; (mov r8, r8)

000802b8 <__retarget_lock_acquire_recursive>:
   802b8:	4770      	bx	lr
   802ba:	46c0      	nop			; (mov r8, r8)

000802bc <__retarget_lock_release_recursive>:
   802bc:	4770      	bx	lr
   802be:	46c0      	nop			; (mov r8, r8)

000802c0 <__register_exitproc>:
   802c0:	b5f0      	push	{r4, r5, r6, r7, lr}
   802c2:	464e      	mov	r6, r9
   802c4:	4645      	mov	r5, r8
   802c6:	46de      	mov	lr, fp
   802c8:	4657      	mov	r7, sl
   802ca:	b5e0      	push	{r5, r6, r7, lr}
   802cc:	4d37      	ldr	r5, [pc, #220]	; (803ac <__register_exitproc+0xec>)
   802ce:	b083      	sub	sp, #12
   802d0:	0006      	movs	r6, r0
   802d2:	6828      	ldr	r0, [r5, #0]
   802d4:	4698      	mov	r8, r3
   802d6:	000f      	movs	r7, r1
   802d8:	4691      	mov	r9, r2
   802da:	f7ff ffed 	bl	802b8 <__retarget_lock_acquire_recursive>
   802de:	4b34      	ldr	r3, [pc, #208]	; (803b0 <__register_exitproc+0xf0>)
   802e0:	681c      	ldr	r4, [r3, #0]
   802e2:	23a4      	movs	r3, #164	; 0xa4
   802e4:	005b      	lsls	r3, r3, #1
   802e6:	58e0      	ldr	r0, [r4, r3]
   802e8:	2800      	cmp	r0, #0
   802ea:	d054      	beq.n	80396 <__register_exitproc+0xd6>
   802ec:	6843      	ldr	r3, [r0, #4]
   802ee:	2b1f      	cmp	r3, #31
   802f0:	dc15      	bgt.n	8031e <__register_exitproc+0x5e>
   802f2:	1c5a      	adds	r2, r3, #1
   802f4:	9201      	str	r2, [sp, #4]
   802f6:	2e00      	cmp	r6, #0
   802f8:	d12a      	bne.n	80350 <__register_exitproc+0x90>
   802fa:	9a01      	ldr	r2, [sp, #4]
   802fc:	3302      	adds	r3, #2
   802fe:	009b      	lsls	r3, r3, #2
   80300:	6042      	str	r2, [r0, #4]
   80302:	501f      	str	r7, [r3, r0]
   80304:	6828      	ldr	r0, [r5, #0]
   80306:	f7ff ffd9 	bl	802bc <__retarget_lock_release_recursive>
   8030a:	2000      	movs	r0, #0
   8030c:	b003      	add	sp, #12
   8030e:	bc3c      	pop	{r2, r3, r4, r5}
   80310:	4690      	mov	r8, r2
   80312:	4699      	mov	r9, r3
   80314:	46a2      	mov	sl, r4
   80316:	46ab      	mov	fp, r5
   80318:	bcf0      	pop	{r4, r5, r6, r7}
   8031a:	bc02      	pop	{r1}
   8031c:	4708      	bx	r1
   8031e:	4b25      	ldr	r3, [pc, #148]	; (803b4 <__register_exitproc+0xf4>)
   80320:	2b00      	cmp	r3, #0
   80322:	d03d      	beq.n	803a0 <__register_exitproc+0xe0>
   80324:	20c8      	movs	r0, #200	; 0xc8
   80326:	0040      	lsls	r0, r0, #1
   80328:	e000      	b.n	8032c <__register_exitproc+0x6c>
   8032a:	bf00      	nop
   8032c:	2800      	cmp	r0, #0
   8032e:	d037      	beq.n	803a0 <__register_exitproc+0xe0>
   80330:	22a4      	movs	r2, #164	; 0xa4
   80332:	2300      	movs	r3, #0
   80334:	0052      	lsls	r2, r2, #1
   80336:	58a1      	ldr	r1, [r4, r2]
   80338:	6043      	str	r3, [r0, #4]
   8033a:	6001      	str	r1, [r0, #0]
   8033c:	50a0      	str	r0, [r4, r2]
   8033e:	3240      	adds	r2, #64	; 0x40
   80340:	5083      	str	r3, [r0, r2]
   80342:	3204      	adds	r2, #4
   80344:	5083      	str	r3, [r0, r2]
   80346:	3301      	adds	r3, #1
   80348:	9301      	str	r3, [sp, #4]
   8034a:	2300      	movs	r3, #0
   8034c:	2e00      	cmp	r6, #0
   8034e:	d0d4      	beq.n	802fa <__register_exitproc+0x3a>
   80350:	009a      	lsls	r2, r3, #2
   80352:	4692      	mov	sl, r2
   80354:	4482      	add	sl, r0
   80356:	464a      	mov	r2, r9
   80358:	2188      	movs	r1, #136	; 0x88
   8035a:	4654      	mov	r4, sl
   8035c:	5062      	str	r2, [r4, r1]
   8035e:	22c4      	movs	r2, #196	; 0xc4
   80360:	0052      	lsls	r2, r2, #1
   80362:	4691      	mov	r9, r2
   80364:	4481      	add	r9, r0
   80366:	464a      	mov	r2, r9
   80368:	3987      	subs	r1, #135	; 0x87
   8036a:	4099      	lsls	r1, r3
   8036c:	6812      	ldr	r2, [r2, #0]
   8036e:	430a      	orrs	r2, r1
   80370:	4694      	mov	ip, r2
   80372:	468b      	mov	fp, r1
   80374:	464a      	mov	r2, r9
   80376:	4661      	mov	r1, ip
   80378:	6011      	str	r1, [r2, #0]
   8037a:	2284      	movs	r2, #132	; 0x84
   8037c:	4641      	mov	r1, r8
   8037e:	0052      	lsls	r2, r2, #1
   80380:	50a1      	str	r1, [r4, r2]
   80382:	2e02      	cmp	r6, #2
   80384:	d1b9      	bne.n	802fa <__register_exitproc+0x3a>
   80386:	0002      	movs	r2, r0
   80388:	465c      	mov	r4, fp
   8038a:	328d      	adds	r2, #141	; 0x8d
   8038c:	32ff      	adds	r2, #255	; 0xff
   8038e:	6811      	ldr	r1, [r2, #0]
   80390:	430c      	orrs	r4, r1
   80392:	6014      	str	r4, [r2, #0]
   80394:	e7b1      	b.n	802fa <__register_exitproc+0x3a>
   80396:	0020      	movs	r0, r4
   80398:	304d      	adds	r0, #77	; 0x4d
   8039a:	30ff      	adds	r0, #255	; 0xff
   8039c:	50e0      	str	r0, [r4, r3]
   8039e:	e7a5      	b.n	802ec <__register_exitproc+0x2c>
   803a0:	6828      	ldr	r0, [r5, #0]
   803a2:	f7ff ff8b 	bl	802bc <__retarget_lock_release_recursive>
   803a6:	2001      	movs	r0, #1
   803a8:	4240      	negs	r0, r0
   803aa:	e7af      	b.n	8030c <__register_exitproc+0x4c>
   803ac:	20000428 	.word	0x20000428
   803b0:	000803b8 	.word	0x000803b8
   803b4:	00000000 	.word	0x00000000

000803b8 <_global_impure_ptr>:
   803b8:	20000000                                ... 

000803bc <_init>:
   803bc:	b5f8      	push	{r3, r4, r5, r6, r7, lr}
   803be:	46c0      	nop			; (mov r8, r8)
   803c0:	bcf8      	pop	{r3, r4, r5, r6, r7}
   803c2:	bc08      	pop	{r3}
   803c4:	469e      	mov	lr, r3
   803c6:	4770      	bx	lr

000803c8 <__init_array_start>:
   803c8:	0008024d 	.word	0x0008024d

000803cc <__frame_dummy_init_array_entry>:
   803cc:	00080121                                !...

000803d0 <_fini>:
   803d0:	b5f8      	push	{r3, r4, r5, r6, r7, lr}
   803d2:	46c0      	nop			; (mov r8, r8)
   803d4:	bcf8      	pop	{r3, r4, r5, r6, r7}
   803d6:	bc08      	pop	{r3}
   803d8:	469e      	mov	lr, r3
   803da:	4770      	bx	lr

000803dc <__fini_array_start>:
   803dc:	000800f5 	.word	0x000800f5
