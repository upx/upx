
tmp/lzma_d_cs.o:     file format elf32-i386

Disassembly of section .text.LzmaDecode:

00000000 <.text.LzmaDecode>:
       0:	push   bp
       1:	mov    bp,sp
       3:	sub    sp,0x92
       7:	push   si
       8:	push   di
       9:	mov    di,word ptr [bp+4]
       c:	mov    ax,di
       e:	add    ax,0x4
      11:	mov    word ptr [bp-112],ax
      14:	mov    word ptr [bp-108],0x0
      19:	mov    word ptr [bp-106],0x0
      1e:	mov    byte ptr [bp-104],0x0
      22:	mov    ax,0x1
      25:	mov    cl,byte ptr [di+2]
      28:	xor    ch,ch
      2a:	shl    ax,cl
      2c:	dec    ax
      2d:	cwd
      2e:	mov    word ptr [bp-98],dx
      31:	mov    word ptr [bp-100],ax
      34:	mov    ax,0x1
      37:	mov    cl,byte ptr [di+1]
      3a:	xor    ch,ch
      3c:	shl    ax,cl
      3e:	dec    ax
      3f:	cwd
      40:	mov    word ptr [bp-94],dx
      43:	mov    word ptr [bp-96],ax
      46:	mov    al,byte ptr [di]
      48:	xor    ah,ah
      4a:	mov    word ptr [bp-92],ax
      4d:	mov    word ptr [bp-90],0x0
      52:	mov    word ptr [bp-88],0x1
      57:	mov    word ptr [bp-86],0x0
      5c:	mov    word ptr [bp-84],0x1
      61:	mov    word ptr [bp-82],0x0
      66:	mov    word ptr [bp-80],0x1
      6b:	mov    word ptr [bp-78],0x0
      70:	mov    word ptr [bp-76],0x1
      75:	mov    word ptr [bp-74],0x0
      7a:	mov    bx,word ptr [bp+14]
      7d:	mov    word ptr [bx],0x0
      81:	mov    word ptr [bx+2],0x0
      86:	mov    bx,word ptr [bp+24]
      89:	mov    word ptr [bx],0x0
      8d:	mov    word ptr [bx+2],0x0
      92:	mov    word ptr [bp-52],0x0
      97:	mov    word ptr [bp-50],0x0
      9c:	mov    bx,0x300
      9f:	xor    dx,dx
      a1:	mov    cl,byte ptr [di+1]
      a4:	xor    ch,ch
      a6:	add    cx,ax
      a8:	jcxz   0xb0
      aa:	shl    bx,1
      ac:	rcl    dx,1
      ae:	loop   0xaa
      b0:	add    bx,0x736
      b4:	adc    dx,cx
      b6:	mov    ax,dx
      b8:	or     ax,bx
      ba:	mov    word ptr [bp-46],dx
      bd:	mov    word ptr [bp-48],bx
      c0:	je     0xe9
      c2:	mov    si,word ptr [bp-52]
      c5:	shl    si,1
      c7:	add    si,word ptr [bp-112]
      ca:	mov    word ptr [si],0x400
      ce:	add    si,0x2
      d1:	add    word ptr [bp-52],0x1
      d5:	adc    word ptr [bp-50],0x0
      d9:	mov    dx,word ptr [bp-50]
      dc:	mov    ax,word ptr [bp-52]
      df:	cmp    dx,word ptr [bp-46]
      e2:	jne    0xe7
      e4:	cmp    ax,word ptr [bp-48]
      e7:	jb     0xca
      e9:	mov    dx,word ptr [bp+8]
      ec:	mov    ax,word ptr [bp+6]
      ef:	mov    word ptr [bp-66],dx
      f2:	mov    word ptr [bp-68],ax
      f5:	mov    cx,dx
      f7:	mov    bx,ax
      f9:	mov    dx,word ptr [bp+12]
      fc:	mov    ax,word ptr [bp+10]
      ff:	add    ax,bx
     101:	mov    bx,cx
     103:	adc    dx,0x0
     106:	mov    cx,0x0	107: R_386_16	__AHSHIFT
     109:	shl    dx,cl
     10b:	add    dx,bx
     10d:	mov    word ptr [bp-62],dx
     110:	mov    word ptr [bp-64],ax
     113:	mov    word ptr [bp-56],0x0
     118:	mov    word ptr [bp-54],0x0
     11d:	mov    word ptr [bp-60],0xffff
     122:	mov    word ptr [bp-58],0xffff
     127:	xor    di,di
     129:	mov    dx,word ptr [bp-66]
     12c:	mov    ax,word ptr [bp-68]
     12f:	cmp    dx,word ptr [bp-62]
     132:	jne    0x137
     134:	cmp    ax,word ptr [bp-64]
     137:	jne    0x13c
     139:	jmp    0x10ee
     13c:	mov    dx,word ptr [bp-54]
     13f:	mov    ax,word ptr [bp-56]
     142:	mov    cx,0x8
     145:	shl    ax,1
     147:	rcl    dx,1
     149:	loop   0x145
     14b:	push   ax
     14c:	push   dx
     14d:	les    bx,dword ptr [bp-68]
     150:	xor    si,si
     152:	add    word ptr [bp-68],0x1
     156:	adc    si,0x0
     159:	mov    cx,0x0	15a: R_386_16	__AHSHIFT
     15c:	shl    si,cl
     15e:	add    word ptr [bp-66],si
     161:	mov    al,byte ptr es:[bx]
     164:	xor    ah,ah
     166:	test   ax,ax
     168:	cwd
     169:	mov    cx,dx
     16b:	mov    bx,ax
     16d:	pop    dx
     16e:	pop    ax
     16f:	or     ax,bx
     171:	or     dx,cx
     173:	mov    word ptr [bp-54],dx
     176:	mov    word ptr [bp-56],ax
     179:	inc    di
     17a:	cmp    di,0x5
     17d:	jb     0x129
     17f:	mov    ax,word ptr [bp+22]
     182:	or     ax,word ptr [bp+20]
     185:	jne    0x18a
     187:	jmp    0x10d1
     18a:	mov    di,word ptr [bp-108]
     18d:	and    di,word ptr [bp-100]
     190:	mov    word ptr [bp-40],di
     193:	shl    di,1
     195:	mov    ax,word ptr [bp-90]
     198:	mov    cx,0x5
     19b:	shl    ax,cl
     19d:	add    ax,word ptr [bp-112]
     1a0:	add    di,ax
     1a2:	cmp    word ptr [bp-58],0x100
     1a7:	jne    0x1ad
     1a9:	cmp    word ptr [bp-60],0x0
     1ad:	jae    0x208
     1af:	mov    dx,word ptr [bp-66]
     1b2:	mov    ax,word ptr [bp-68]
     1b5:	cmp    dx,word ptr [bp-62]
     1b8:	jne    0x1bd
     1ba:	cmp    ax,word ptr [bp-64]
     1bd:	jne    0x1c2
     1bf:	jmp    0x10ee
     1c2:	mov    cl,0x8
     1c4:	shl    word ptr [bp-60],1
     1c7:	rcl    word ptr [bp-58],1
     1ca:	loop   0x1c4
     1cc:	mov    dx,word ptr [bp-54]
     1cf:	mov    ax,word ptr [bp-56]
     1d2:	mov    cl,0x8
     1d4:	shl    ax,1
     1d6:	rcl    dx,1
     1d8:	loop   0x1d4
     1da:	push   ax
     1db:	push   dx
     1dc:	les    bx,dword ptr [bp-68]
     1df:	xor    si,si
     1e1:	add    word ptr [bp-68],0x1
     1e5:	adc    si,0x0
     1e8:	mov    cx,0x0	1e9: R_386_16	__AHSHIFT
     1eb:	shl    si,cl
     1ed:	add    word ptr [bp-66],si
     1f0:	mov    al,byte ptr es:[bx]
     1f3:	xor    ah,ah
     1f5:	test   ax,ax
     1f7:	cwd
     1f8:	mov    cx,dx
     1fa:	mov    bx,ax
     1fc:	pop    dx
     1fd:	pop    ax
     1fe:	or     ax,bx
     200:	or     dx,cx
     202:	mov    word ptr [bp-54],dx
     205:	mov    word ptr [bp-56],ax
     208:	mov    dx,word ptr [bp-58]
     20b:	mov    ax,word ptr [bp-60]
     20e:	mov    cx,0xb
     211:	shr    dx,1
     213:	rcr    ax,1
     215:	loop   0x211
     217:	mov    bx,word ptr [di]
     219:	xor    cx,cx
     21b:	call   0x21c	21c: R_386_PC16	__LMUL
     21e:	mov    word ptr [bp-42],dx
     221:	mov    word ptr [bp-44],ax
     224:	cmp    dx,word ptr [bp-54]
     227:	jne    0x22c
     229:	cmp    ax,word ptr [bp-56]
     22c:	ja     0x231
     22e:	jmp    0x500
     231:	mov    word ptr [bp-38],0x1
     236:	mov    dx,word ptr [bp-42]
     239:	mov    ax,word ptr [bp-44]
     23c:	mov    word ptr [bp-58],dx
     23f:	mov    word ptr [bp-60],ax
     242:	mov    ax,0x800
     245:	sub    ax,word ptr [di]
     247:	mov    cx,0x5
     24a:	shr    ax,cl
     24c:	add    word ptr [di],ax
     24e:	mov    ax,word ptr [bp-108]
     251:	and    ax,word ptr [bp-96]
     254:	mov    cx,word ptr [bp-92]
     257:	shl    ax,cl
     259:	mov    dl,byte ptr [bp-104]
     25c:	xor    dh,dh
     25e:	mov    cx,0x8
     261:	sub    cx,word ptr [bp-92]
     264:	sar    dx,cl
     266:	add    ax,dx
     268:	mov    cx,0x300
     26b:	mul    cx
     26d:	mov    di,ax
     26f:	shl    di,1
     271:	add    di,word ptr [bp-112]
     274:	add    di,0xe6c
     278:	cmp    word ptr [bp-90],0x7
     27c:	jge    0x281
     27e:	jmp    0x3b1
     281:	mov    dx,word ptr [bp-106]
     284:	mov    bx,word ptr [bp-108]
     287:	mov    cx,word ptr [bp+18]
     28a:	mov    ax,word ptr [bp+16]
     28d:	add    bx,ax
     28f:	mov    ax,cx
     291:	adc    dx,0x0
     294:	mov    cx,0x0	295: R_386_16	__AHSHIFT
     297:	shl    dx,cl
     299:	add    dx,ax
     29b:	sub    bx,word ptr [bp-88]
     29e:	sbb    dx,word ptr [bp-86]
     2a1:	movl   es,dx
     2a3:	mov    al,byte ptr es:[bx]
     2a6:	xor    ah,ah
     2a8:	mov    word ptr [bp-36],ax
     2ab:	mov    dx,word ptr [bp-36]
     2ae:	shl    dx,1
     2b0:	mov    word ptr [bp-36],dx
     2b3:	mov    si,word ptr [bp-38]
     2b6:	shl    si,1
     2b8:	and    dx,0x100
     2bc:	mov    word ptr [bp-34],dx
     2bf:	shl    dx,1
     2c1:	add    dx,di
     2c3:	add    si,dx
     2c5:	add    si,0x200
     2c9:	cmp    word ptr [bp-58],0x100
     2ce:	jne    0x2d4
     2d0:	cmp    word ptr [bp-60],0x0
     2d4:	jae    0x336
     2d6:	mov    dx,word ptr [bp-66]
     2d9:	mov    ax,word ptr [bp-68]
     2dc:	cmp    dx,word ptr [bp-62]
     2df:	jne    0x2e4
     2e1:	cmp    ax,word ptr [bp-64]
     2e4:	jne    0x2e9
     2e6:	jmp    0x10ee
     2e9:	mov    word ptr [bp-32],si
     2ec:	mov    cx,0x8
     2ef:	shl    word ptr [bp-60],1
     2f2:	rcl    word ptr [bp-58],1
     2f5:	loop   0x2ef
     2f7:	mov    dx,word ptr [bp-54]
     2fa:	mov    ax,word ptr [bp-56]
     2fd:	mov    cl,0x8
     2ff:	shl    ax,1
     301:	rcl    dx,1
     303:	loop   0x2ff
     305:	push   ax
     306:	push   dx
     307:	les    bx,dword ptr [bp-68]
     30a:	xor    si,si
     30c:	add    word ptr [bp-68],0x1
     310:	adc    si,0x0
     313:	mov    cx,0x0	314: R_386_16	__AHSHIFT
     316:	shl    si,cl
     318:	add    word ptr [bp-66],si
     31b:	mov    al,byte ptr es:[bx]
     31e:	xor    ah,ah
     320:	test   ax,ax
     322:	cwd
     323:	mov    cx,dx
     325:	mov    bx,ax
     327:	pop    dx
     328:	pop    ax
     329:	or     ax,bx
     32b:	or     dx,cx
     32d:	mov    word ptr [bp-54],dx
     330:	mov    word ptr [bp-56],ax
     333:	mov    si,word ptr [bp-32]
     336:	mov    dx,word ptr [bp-58]
     339:	mov    ax,word ptr [bp-60]
     33c:	mov    cx,0xb
     33f:	shr    dx,1
     341:	rcr    ax,1
     343:	loop   0x33f
     345:	mov    bx,word ptr [si]
     347:	mov    word ptr [bp-116],bx
     34a:	xor    cx,cx
     34c:	call   0x34d	34d: R_386_PC16	__LMUL
     34f:	mov    word ptr [bp-42],dx
     352:	mov    word ptr [bp-44],ax
     355:	cmp    dx,word ptr [bp-54]
     358:	jne    0x35d
     35a:	cmp    ax,word ptr [bp-56]
     35d:	jbe    0x382
     35f:	mov    dx,word ptr [bp-42]
     362:	mov    ax,word ptr [bp-44]
     365:	mov    word ptr [bp-58],dx
     368:	mov    word ptr [bp-60],ax
     36b:	mov    dx,0x800
     36e:	sub    dx,word ptr [si]
     370:	mov    cx,0x5
     373:	shr    dx,cl
     375:	add    word ptr [si],dx
     377:	shl    word ptr [bp-38],1
     37a:	cmp    word ptr [bp-34],0x0
     37e:	jne    0x3b1
     380:	jmp    0x3a7
     382:	sub    word ptr [bp-60],ax
     385:	sbb    word ptr [bp-58],dx
     388:	sub    word ptr [bp-56],ax
     38b:	sbb    word ptr [bp-54],dx
     38e:	mov    dx,word ptr [bp-116]
     391:	mov    cx,0x5
     394:	shr    dx,cl
     396:	sub    word ptr [si],dx
     398:	mov    cx,word ptr [bp-38]
     39b:	shl    cx,1
     39d:	inc    cx
     39e:	mov    word ptr [bp-38],cx
     3a1:	cmp    word ptr [bp-34],0x0
     3a5:	je     0x3b1
     3a7:	cmp    word ptr [bp-38],0x100
     3ac:	jge    0x3b1
     3ae:	jmp    0x2ab
     3b1:	cmp    word ptr [bp-38],0x100
     3b6:	jl     0x3bb
     3b8:	jmp    0x4a0
     3bb:	mov    si,word ptr [bp-38]
     3be:	shl    si,1
     3c0:	add    si,di
     3c2:	cmp    word ptr [bp-58],0x100
     3c7:	jne    0x3cd
     3c9:	cmp    word ptr [bp-60],0x0
     3cd:	jae    0x42f
     3cf:	mov    dx,word ptr [bp-66]
     3d2:	mov    ax,word ptr [bp-68]
     3d5:	cmp    dx,word ptr [bp-62]
     3d8:	jne    0x3dd
     3da:	cmp    ax,word ptr [bp-64]
     3dd:	jne    0x3e2
     3df:	jmp    0x10ee
     3e2:	mov    word ptr [bp-30],si
     3e5:	mov    cx,0x8
     3e8:	shl    word ptr [bp-60],1
     3eb:	rcl    word ptr [bp-58],1
     3ee:	loop   0x3e8
     3f0:	mov    dx,word ptr [bp-54]
     3f3:	mov    ax,word ptr [bp-56]
     3f6:	mov    cl,0x8
     3f8:	shl    ax,1
     3fa:	rcl    dx,1
     3fc:	loop   0x3f8
     3fe:	push   ax
     3ff:	push   dx
     400:	les    bx,dword ptr [bp-68]
     403:	xor    si,si
     405:	add    word ptr [bp-68],0x1
     409:	adc    si,0x0
     40c:	mov    cx,0x0	40d: R_386_16	__AHSHIFT
     40f:	shl    si,cl
     411:	add    word ptr [bp-66],si
     414:	mov    al,byte ptr es:[bx]
     417:	xor    ah,ah
     419:	test   ax,ax
     41b:	cwd
     41c:	mov    cx,dx
     41e:	mov    bx,ax
     420:	pop    dx
     421:	pop    ax
     422:	or     ax,bx
     424:	or     dx,cx
     426:	mov    word ptr [bp-54],dx
     429:	mov    word ptr [bp-56],ax
     42c:	mov    si,word ptr [bp-30]
     42f:	mov    dx,word ptr [bp-58]
     432:	mov    ax,word ptr [bp-60]
     435:	mov    cx,0xb
     438:	shr    dx,1
     43a:	rcr    ax,1
     43c:	loop   0x438
     43e:	mov    bx,word ptr [si]
     440:	mov    word ptr [bp-114],bx
     443:	xor    cx,cx
     445:	call   0x446	446: R_386_PC16	__LMUL
     448:	mov    word ptr [bp-42],dx
     44b:	mov    word ptr [bp-44],ax
     44e:	cmp    dx,word ptr [bp-54]
     451:	jne    0x456
     453:	cmp    ax,word ptr [bp-56]
     456:	jbe    0x47f
     458:	mov    dx,word ptr [bp-42]
     45b:	mov    ax,word ptr [bp-44]
     45e:	mov    word ptr [bp-58],dx
     461:	mov    word ptr [bp-60],ax
     464:	mov    dx,0x800
     467:	sub    dx,word ptr [si]
     469:	mov    cx,0x5
     46c:	shr    dx,cl
     46e:	add    word ptr [si],dx
     470:	shl    word ptr [bp-38],1
     473:	cmp    word ptr [bp-38],0x100
     478:	jge    0x47d
     47a:	jmp    0x3bb
     47d:	jmp    0x4a0
     47f:	sub    word ptr [bp-60],ax
     482:	sbb    word ptr [bp-58],dx
     485:	sub    word ptr [bp-56],ax
     488:	sbb    word ptr [bp-54],dx
     48b:	mov    dx,word ptr [bp-114]
     48e:	mov    cx,0x5
     491:	shr    dx,cl
     493:	sub    word ptr [si],dx
     495:	mov    cx,word ptr [bp-38]
     498:	shl    cx,1
     49a:	inc    cx
     49b:	mov    word ptr [bp-38],cx
     49e:	jmp    0x473
     4a0:	mov    al,byte ptr [bp-38]
     4a3:	mov    byte ptr [bp-104],al
     4a6:	mov    bx,word ptr [bp-108]
     4a9:	mov    dx,word ptr [bp-106]
     4ac:	add    word ptr [bp-108],0x1
     4b0:	adc    word ptr [bp-106],0x0
     4b4:	mov    cx,word ptr [bp+18]
     4b7:	mov    si,word ptr [bp+16]
     4ba:	add    bx,si
     4bc:	mov    si,cx
     4be:	adc    dx,0x0
     4c1:	mov    cx,0x0	4c2: R_386_16	__AHSHIFT
     4c4:	shl    dx,cl
     4c6:	add    dx,si
     4c8:	movl   es,dx
     4ca:	mov    byte ptr es:[bx],al
     4cd:	cmp    word ptr [bp-90],0x4
     4d1:	jge    0x4ee
     4d3:	mov    word ptr [bp-90],0x0
     4d8:	mov    dx,word ptr [bp-106]
     4db:	mov    ax,word ptr [bp-108]
     4de:	cmp    dx,word ptr [bp+22]
     4e1:	jne    0x4e6
     4e3:	cmp    ax,word ptr [bp+20]
     4e6:	jae    0x4eb
     4e8:	jmp    0x18a
     4eb:	jmp    0x10d1
     4ee:	cmp    word ptr [bp-90],0xa
     4f2:	jge    0x4fa
     4f4:	add    word ptr [bp-90],0xfffffffd
     4f8:	jmp    0x4d8
     4fa:	add    word ptr [bp-90],0xfffffffa
     4fe:	jmp    0x4d8
     500:	sub    word ptr [bp-60],ax
     503:	sbb    word ptr [bp-58],dx
     506:	sub    word ptr [bp-56],ax
     509:	sbb    word ptr [bp-54],dx
     50c:	mov    ax,word ptr [di]
     50e:	mov    cx,0x5
     511:	shr    ax,cl
     513:	sub    word ptr [di],ax
     515:	mov    di,word ptr [bp-90]
     518:	shl    di,1
     51a:	add    di,word ptr [bp-112]
     51d:	add    di,0x180
     521:	cmp    word ptr [bp-58],0x100
     526:	jne    0x52c
     528:	cmp    word ptr [bp-60],0x0
     52c:	jae    0x587
     52e:	mov    dx,word ptr [bp-66]
     531:	mov    ax,word ptr [bp-68]
     534:	cmp    dx,word ptr [bp-62]
     537:	jne    0x53c
     539:	cmp    ax,word ptr [bp-64]
     53c:	jne    0x541
     53e:	jmp    0x10ee
     541:	mov    cl,0x8
     543:	shl    word ptr [bp-60],1
     546:	rcl    word ptr [bp-58],1
     549:	loop   0x543
     54b:	mov    dx,word ptr [bp-54]
     54e:	mov    ax,word ptr [bp-56]
     551:	mov    cl,0x8
     553:	shl    ax,1
     555:	rcl    dx,1
     557:	loop   0x553
     559:	push   ax
     55a:	push   dx
     55b:	les    bx,dword ptr [bp-68]
     55e:	xor    si,si
     560:	add    word ptr [bp-68],0x1
     564:	adc    si,0x0
     567:	mov    cx,0x0	568: R_386_16	__AHSHIFT
     56a:	shl    si,cl
     56c:	add    word ptr [bp-66],si
     56f:	mov    al,byte ptr es:[bx]
     572:	xor    ah,ah
     574:	test   ax,ax
     576:	cwd
     577:	mov    cx,dx
     579:	mov    bx,ax
     57b:	pop    dx
     57c:	pop    ax
     57d:	or     ax,bx
     57f:	or     dx,cx
     581:	mov    word ptr [bp-54],dx
     584:	mov    word ptr [bp-56],ax
     587:	mov    dx,word ptr [bp-58]
     58a:	mov    ax,word ptr [bp-60]
     58d:	mov    cx,0xb
     590:	shr    dx,1
     592:	rcr    ax,1
     594:	loop   0x590
     596:	mov    bx,word ptr [di]
     598:	xor    cx,cx
     59a:	call   0x59b	59b: R_386_PC16	__LMUL
     59d:	mov    word ptr [bp-42],dx
     5a0:	mov    word ptr [bp-44],ax
     5a3:	cmp    dx,word ptr [bp-54]
     5a6:	jne    0x5ab
     5a8:	cmp    ax,word ptr [bp-56]
     5ab:	jbe    0x601
     5ad:	mov    dx,word ptr [bp-42]
     5b0:	mov    ax,word ptr [bp-44]
     5b3:	mov    word ptr [bp-58],dx
     5b6:	mov    word ptr [bp-60],ax
     5b9:	mov    ax,0x800
     5bc:	sub    ax,word ptr [di]
     5be:	mov    cx,0x5
     5c1:	shr    ax,cl
     5c3:	add    word ptr [di],ax
     5c5:	mov    dx,word ptr [bp-78]
     5c8:	mov    ax,word ptr [bp-80]
     5cb:	mov    word ptr [bp-74],dx
     5ce:	mov    word ptr [bp-76],ax
     5d1:	mov    dx,word ptr [bp-82]
     5d4:	mov    ax,word ptr [bp-84]
     5d7:	mov    word ptr [bp-78],dx
     5da:	mov    word ptr [bp-80],ax
     5dd:	mov    dx,word ptr [bp-86]
     5e0:	mov    ax,word ptr [bp-88]
     5e3:	mov    word ptr [bp-82],dx
     5e6:	mov    word ptr [bp-84],ax
     5e9:	cmp    word ptr [bp-90],0x7
     5ed:	mov    ax,0x0
     5f0:	jl     0x5f4
     5f2:	mov    al,0x3
     5f4:	mov    word ptr [bp-90],ax
     5f7:	mov    di,word ptr [bp-112]
     5fa:	add    di,0x664
     5fe:	jmp    0xa10
     601:	sub    word ptr [bp-60],ax
     604:	sbb    word ptr [bp-58],dx
     607:	sub    word ptr [bp-56],ax
     60a:	sbb    word ptr [bp-54],dx
     60d:	mov    ax,word ptr [di]
     60f:	mov    cx,0x5
     612:	shr    ax,cl
     614:	sub    word ptr [di],ax
     616:	mov    di,word ptr [bp-90]
     619:	shl    di,1
     61b:	add    di,word ptr [bp-112]
     61e:	add    di,0x198
     622:	cmp    word ptr [bp-58],0x100
     627:	jne    0x62d
     629:	cmp    word ptr [bp-60],0x0
     62d:	jae    0x688
     62f:	mov    dx,word ptr [bp-66]
     632:	mov    ax,word ptr [bp-68]
     635:	cmp    dx,word ptr [bp-62]
     638:	jne    0x63d
     63a:	cmp    ax,word ptr [bp-64]
     63d:	jne    0x642
     63f:	jmp    0x10ee
     642:	mov    cl,0x8
     644:	shl    word ptr [bp-60],1
     647:	rcl    word ptr [bp-58],1
     64a:	loop   0x644
     64c:	mov    dx,word ptr [bp-54]
     64f:	mov    ax,word ptr [bp-56]
     652:	mov    cl,0x8
     654:	shl    ax,1
     656:	rcl    dx,1
     658:	loop   0x654
     65a:	push   ax
     65b:	push   dx
     65c:	les    bx,dword ptr [bp-68]
     65f:	xor    si,si
     661:	add    word ptr [bp-68],0x1
     665:	adc    si,0x0
     668:	mov    cx,0x0	669: R_386_16	__AHSHIFT
     66b:	shl    si,cl
     66d:	add    word ptr [bp-66],si
     670:	mov    al,byte ptr es:[bx]
     673:	xor    ah,ah
     675:	test   ax,ax
     677:	cwd
     678:	mov    cx,dx
     67a:	mov    bx,ax
     67c:	pop    dx
     67d:	pop    ax
     67e:	or     ax,bx
     680:	or     dx,cx
     682:	mov    word ptr [bp-54],dx
     685:	mov    word ptr [bp-56],ax
     688:	mov    dx,word ptr [bp-58]
     68b:	mov    ax,word ptr [bp-60]
     68e:	mov    cx,0xb
     691:	shr    dx,1
     693:	rcr    ax,1
     695:	loop   0x691
     697:	mov    bx,word ptr [di]
     699:	xor    cx,cx
     69b:	call   0x69c	69c: R_386_PC16	__LMUL
     69e:	mov    word ptr [bp-42],dx
     6a1:	mov    word ptr [bp-44],ax
     6a4:	cmp    dx,word ptr [bp-54]
     6a7:	jne    0x6ac
     6a9:	cmp    ax,word ptr [bp-56]
     6ac:	ja     0x6b1
     6ae:	jmp    0x803
     6b1:	mov    dx,word ptr [bp-42]
     6b4:	mov    ax,word ptr [bp-44]
     6b7:	mov    word ptr [bp-58],dx
     6ba:	mov    word ptr [bp-60],ax
     6bd:	mov    bx,0x800
     6c0:	sub    bx,word ptr [di]
     6c2:	mov    cx,0x5
     6c5:	shr    bx,cl
     6c7:	add    word ptr [di],bx
     6c9:	mov    di,word ptr [bp-40]
     6cc:	shl    di,1
     6ce:	mov    bx,word ptr [bp-90]
     6d1:	shl    bx,cl
     6d3:	add    bx,word ptr [bp-112]
     6d6:	add    di,bx
     6d8:	add    di,0x1e0
     6dc:	cmp    dx,0x100
     6e0:	jne    0x6e4
     6e2:	test   ax,ax
     6e4:	jae    0x73f
     6e6:	mov    dx,word ptr [bp-66]
     6e9:	mov    ax,word ptr [bp-68]
     6ec:	cmp    dx,word ptr [bp-62]
     6ef:	jne    0x6f4
     6f1:	cmp    ax,word ptr [bp-64]
     6f4:	jne    0x6f9
     6f6:	jmp    0x10ee
     6f9:	mov    cl,0x8
     6fb:	shl    word ptr [bp-60],1
     6fe:	rcl    word ptr [bp-58],1
     701:	loop   0x6fb
     703:	mov    dx,word ptr [bp-54]
     706:	mov    ax,word ptr [bp-56]
     709:	mov    cl,0x8
     70b:	shl    ax,1
     70d:	rcl    dx,1
     70f:	loop   0x70b
     711:	push   ax
     712:	push   dx
     713:	les    bx,dword ptr [bp-68]
     716:	xor    si,si
     718:	add    word ptr [bp-68],0x1
     71c:	adc    si,0x0
     71f:	mov    cx,0x0	720: R_386_16	__AHSHIFT
     722:	shl    si,cl
     724:	add    word ptr [bp-66],si
     727:	mov    al,byte ptr es:[bx]
     72a:	xor    ah,ah
     72c:	test   ax,ax
     72e:	cwd
     72f:	mov    cx,dx
     731:	mov    bx,ax
     733:	pop    dx
     734:	pop    ax
     735:	or     ax,bx
     737:	or     dx,cx
     739:	mov    word ptr [bp-54],dx
     73c:	mov    word ptr [bp-56],ax
     73f:	mov    dx,word ptr [bp-58]
     742:	mov    ax,word ptr [bp-60]
     745:	mov    cx,0xb
     748:	shr    dx,1
     74a:	rcr    ax,1
     74c:	loop   0x748
     74e:	mov    bx,word ptr [di]
     750:	xor    cx,cx
     752:	call   0x753	753: R_386_PC16	__LMUL
     755:	mov    word ptr [bp-42],dx
     758:	mov    word ptr [bp-44],ax
     75b:	cmp    dx,word ptr [bp-54]
     75e:	jne    0x763
     760:	cmp    ax,word ptr [bp-56]
     763:	ja     0x768
     765:	jmp    0x7eb
     768:	mov    dx,word ptr [bp-42]
     76b:	mov    ax,word ptr [bp-44]
     76e:	mov    word ptr [bp-58],dx
     771:	mov    word ptr [bp-60],ax
     774:	mov    ax,0x800
     777:	sub    ax,word ptr [di]
     779:	mov    cx,0x5
     77c:	shr    ax,cl
     77e:	add    word ptr [di],ax
     780:	mov    ax,word ptr [bp-106]
     783:	or     ax,word ptr [bp-108]
     786:	jne    0x78b
     788:	jmp    0x10ee
     78b:	cmp    word ptr [bp-90],0x7
     78f:	mov    ax,0x9
     792:	jl     0x796
     794:	mov    al,0xb
     796:	mov    word ptr [bp-90],ax
     799:	mov    dx,word ptr [bp-106]
     79c:	mov    bx,word ptr [bp-108]
     79f:	mov    cx,word ptr [bp+18]
     7a2:	mov    ax,word ptr [bp+16]
     7a5:	add    bx,ax
     7a7:	mov    ax,cx
     7a9:	adc    dx,0x0
     7ac:	mov    cx,0x0	7ad: R_386_16	__AHSHIFT
     7af:	shl    dx,cl
     7b1:	add    dx,ax
     7b3:	sub    bx,word ptr [bp-88]
     7b6:	sbb    dx,word ptr [bp-86]
     7b9:	movl   es,dx
     7bb:	mov    al,byte ptr es:[bx]
     7be:	mov    byte ptr [bp-104],al
     7c1:	mov    bx,word ptr [bp-108]
     7c4:	mov    dx,word ptr [bp-106]
     7c7:	add    word ptr [bp-108],0x1
     7cb:	adc    word ptr [bp-106],0x0
     7cf:	mov    cx,word ptr [bp+18]
     7d2:	mov    si,word ptr [bp+16]
     7d5:	add    bx,si
     7d7:	mov    si,cx
     7d9:	adc    dx,0x0
     7dc:	mov    cx,0x0	7dd: R_386_16	__AHSHIFT
     7df:	shl    dx,cl
     7e1:	add    dx,si
     7e3:	movl   es,dx
     7e5:	mov    byte ptr es:[bx],al
     7e8:	jmp    0x4d8
     7eb:	sub    word ptr [bp-60],ax
     7ee:	sbb    word ptr [bp-58],dx
     7f1:	sub    word ptr [bp-56],ax
     7f4:	sbb    word ptr [bp-54],dx
     7f7:	mov    ax,word ptr [di]
     7f9:	mov    cx,0x5
     7fc:	shr    ax,cl
     7fe:	sub    word ptr [di],ax
     800:	jmp    0x9fb
     803:	sub    word ptr [bp-60],ax
     806:	sbb    word ptr [bp-58],dx
     809:	sub    word ptr [bp-56],ax
     80c:	sbb    word ptr [bp-54],dx
     80f:	mov    ax,word ptr [di]
     811:	mov    cx,0x5
     814:	shr    ax,cl
     816:	sub    word ptr [di],ax
     818:	mov    di,word ptr [bp-90]
     81b:	shl    di,1
     81d:	add    di,word ptr [bp-112]
     820:	add    di,0x1b0
     824:	cmp    word ptr [bp-58],0x100
     829:	jne    0x82f
     82b:	cmp    word ptr [bp-60],0x0
     82f:	jae    0x88a
     831:	mov    dx,word ptr [bp-66]
     834:	mov    ax,word ptr [bp-68]
     837:	cmp    dx,word ptr [bp-62]
     83a:	jne    0x83f
     83c:	cmp    ax,word ptr [bp-64]
     83f:	jne    0x844
     841:	jmp    0x10ee
     844:	mov    cl,0x8
     846:	shl    word ptr [bp-60],1
     849:	rcl    word ptr [bp-58],1
     84c:	loop   0x846
     84e:	mov    dx,word ptr [bp-54]
     851:	mov    ax,word ptr [bp-56]
     854:	mov    cl,0x8
     856:	shl    ax,1
     858:	rcl    dx,1
     85a:	loop   0x856
     85c:	push   ax
     85d:	push   dx
     85e:	les    bx,dword ptr [bp-68]
     861:	xor    si,si
     863:	add    word ptr [bp-68],0x1
     867:	adc    si,0x0
     86a:	mov    cx,0x0	86b: R_386_16	__AHSHIFT
     86d:	shl    si,cl
     86f:	add    word ptr [bp-66],si
     872:	mov    al,byte ptr es:[bx]
     875:	xor    ah,ah
     877:	test   ax,ax
     879:	cwd
     87a:	mov    cx,dx
     87c:	mov    bx,ax
     87e:	pop    dx
     87f:	pop    ax
     880:	or     ax,bx
     882:	or     dx,cx
     884:	mov    word ptr [bp-54],dx
     887:	mov    word ptr [bp-56],ax
     88a:	mov    dx,word ptr [bp-58]
     88d:	mov    ax,word ptr [bp-60]
     890:	mov    cx,0xb
     893:	shr    dx,1
     895:	rcr    ax,1
     897:	loop   0x893
     899:	mov    bx,word ptr [di]
     89b:	xor    cx,cx
     89d:	call   0x89e	89e: R_386_PC16	__LMUL
     8a0:	mov    word ptr [bp-42],dx
     8a3:	mov    word ptr [bp-44],ax
     8a6:	cmp    dx,word ptr [bp-54]
     8a9:	jne    0x8ae
     8ab:	cmp    ax,word ptr [bp-56]
     8ae:	jbe    0x8d7
     8b0:	mov    dx,word ptr [bp-42]
     8b3:	mov    ax,word ptr [bp-44]
     8b6:	mov    word ptr [bp-58],dx
     8b9:	mov    word ptr [bp-60],ax
     8bc:	mov    ax,0x800
     8bf:	sub    ax,word ptr [di]
     8c1:	mov    cx,0x5
     8c4:	shr    ax,cl
     8c6:	add    word ptr [di],ax
     8c8:	mov    dx,word ptr [bp-82]
     8cb:	mov    ax,word ptr [bp-84]
     8ce:	mov    word ptr [bp-26],dx
     8d1:	mov    word ptr [bp-28],ax
     8d4:	jmp    0x9e3
     8d7:	sub    word ptr [bp-60],ax
     8da:	sbb    word ptr [bp-58],dx
     8dd:	sub    word ptr [bp-56],ax
     8e0:	sbb    word ptr [bp-54],dx
     8e3:	mov    ax,word ptr [di]
     8e5:	mov    cx,0x5
     8e8:	shr    ax,cl
     8ea:	sub    word ptr [di],ax
     8ec:	mov    di,word ptr [bp-90]
     8ef:	shl    di,1
     8f1:	add    di,word ptr [bp-112]
     8f4:	add    di,0x1c8
     8f8:	cmp    word ptr [bp-58],0x100
     8fd:	jne    0x903
     8ff:	cmp    word ptr [bp-60],0x0
     903:	jae    0x95e
     905:	mov    dx,word ptr [bp-66]
     908:	mov    ax,word ptr [bp-68]
     90b:	cmp    dx,word ptr [bp-62]
     90e:	jne    0x913
     910:	cmp    ax,word ptr [bp-64]
     913:	jne    0x918
     915:	jmp    0x10ee
     918:	mov    cl,0x8
     91a:	shl    word ptr [bp-60],1
     91d:	rcl    word ptr [bp-58],1
     920:	loop   0x91a
     922:	mov    dx,word ptr [bp-54]
     925:	mov    ax,word ptr [bp-56]
     928:	mov    cl,0x8
     92a:	shl    ax,1
     92c:	rcl    dx,1
     92e:	loop   0x92a
     930:	push   ax
     931:	push   dx
     932:	les    bx,dword ptr [bp-68]
     935:	xor    si,si
     937:	add    word ptr [bp-68],0x1
     93b:	adc    si,0x0
     93e:	mov    cx,0x0	93f: R_386_16	__AHSHIFT
     941:	shl    si,cl
     943:	add    word ptr [bp-66],si
     946:	mov    al,byte ptr es:[bx]
     949:	xor    ah,ah
     94b:	test   ax,ax
     94d:	cwd
     94e:	mov    cx,dx
     950:	mov    bx,ax
     952:	pop    dx
     953:	pop    ax
     954:	or     ax,bx
     956:	or     dx,cx
     958:	mov    word ptr [bp-54],dx
     95b:	mov    word ptr [bp-56],ax
     95e:	mov    dx,word ptr [bp-58]
     961:	mov    ax,word ptr [bp-60]
     964:	mov    cx,0xb
     967:	shr    dx,1
     969:	rcr    ax,1
     96b:	loop   0x967
     96d:	mov    bx,word ptr [di]
     96f:	xor    cx,cx
     971:	call   0x972	972: R_386_PC16	__LMUL
     974:	mov    word ptr [bp-42],dx
     977:	mov    word ptr [bp-44],ax
     97a:	cmp    dx,word ptr [bp-54]
     97d:	jne    0x982
     97f:	cmp    ax,word ptr [bp-56]
     982:	jbe    0x9aa
     984:	mov    dx,word ptr [bp-42]
     987:	mov    ax,word ptr [bp-44]
     98a:	mov    word ptr [bp-58],dx
     98d:	mov    word ptr [bp-60],ax
     990:	mov    ax,0x800
     993:	sub    ax,word ptr [di]
     995:	mov    cx,0x5
     998:	shr    ax,cl
     99a:	add    word ptr [di],ax
     99c:	mov    dx,word ptr [bp-78]
     99f:	mov    ax,word ptr [bp-80]
     9a2:	mov    word ptr [bp-26],dx
     9a5:	mov    word ptr [bp-28],ax
     9a8:	jmp    0x9d7
     9aa:	sub    word ptr [bp-60],ax
     9ad:	sbb    word ptr [bp-58],dx
     9b0:	sub    word ptr [bp-56],ax
     9b3:	sbb    word ptr [bp-54],dx
     9b6:	mov    ax,word ptr [di]
     9b8:	mov    cx,0x5
     9bb:	shr    ax,cl
     9bd:	sub    word ptr [di],ax
     9bf:	mov    dx,word ptr [bp-74]
     9c2:	mov    ax,word ptr [bp-76]
     9c5:	mov    word ptr [bp-26],dx
     9c8:	mov    word ptr [bp-28],ax
     9cb:	mov    dx,word ptr [bp-78]
     9ce:	mov    ax,word ptr [bp-80]
     9d1:	mov    word ptr [bp-74],dx
     9d4:	mov    word ptr [bp-76],ax
     9d7:	mov    dx,word ptr [bp-82]
     9da:	mov    ax,word ptr [bp-84]
     9dd:	mov    word ptr [bp-78],dx
     9e0:	mov    word ptr [bp-80],ax
     9e3:	mov    dx,word ptr [bp-86]
     9e6:	mov    ax,word ptr [bp-88]
     9e9:	mov    word ptr [bp-82],dx
     9ec:	mov    word ptr [bp-84],ax
     9ef:	mov    dx,word ptr [bp-26]
     9f2:	mov    ax,word ptr [bp-28]
     9f5:	mov    word ptr [bp-86],dx
     9f8:	mov    word ptr [bp-88],ax
     9fb:	cmp    word ptr [bp-90],0x7
     9ff:	mov    ax,0x8
     a02:	jl     0xa06
     a04:	mov    al,0xb
     a06:	mov    word ptr [bp-90],ax
     a09:	mov    di,word ptr [bp-112]
     a0c:	add    di,0xa68
     a10:	cmp    word ptr [bp-58],0x100
     a15:	jne    0xa1b
     a17:	cmp    word ptr [bp-60],0x0
     a1b:	jae    0xa76
     a1d:	mov    dx,word ptr [bp-66]
     a20:	mov    ax,word ptr [bp-68]
     a23:	cmp    dx,word ptr [bp-62]
     a26:	jne    0xa2b
     a28:	cmp    ax,word ptr [bp-64]
     a2b:	jne    0xa30
     a2d:	jmp    0x10ee
     a30:	mov    cl,0x8
     a32:	shl    word ptr [bp-60],1
     a35:	rcl    word ptr [bp-58],1
     a38:	loop   0xa32
     a3a:	mov    dx,word ptr [bp-54]
     a3d:	mov    ax,word ptr [bp-56]
     a40:	mov    cl,0x8
     a42:	shl    ax,1
     a44:	rcl    dx,1
     a46:	loop   0xa42
     a48:	push   ax
     a49:	push   dx
     a4a:	les    bx,dword ptr [bp-68]
     a4d:	xor    si,si
     a4f:	add    word ptr [bp-68],0x1
     a53:	adc    si,0x0
     a56:	mov    cx,0x0	a57: R_386_16	__AHSHIFT
     a59:	shl    si,cl
     a5b:	add    word ptr [bp-66],si
     a5e:	mov    al,byte ptr es:[bx]
     a61:	xor    ah,ah
     a63:	test   ax,ax
     a65:	cwd
     a66:	mov    cx,dx
     a68:	mov    bx,ax
     a6a:	pop    dx
     a6b:	pop    ax
     a6c:	or     ax,bx
     a6e:	or     dx,cx
     a70:	mov    word ptr [bp-54],dx
     a73:	mov    word ptr [bp-56],ax
     a76:	mov    dx,word ptr [bp-58]
     a79:	mov    ax,word ptr [bp-60]
     a7c:	mov    cx,0xb
     a7f:	shr    dx,1
     a81:	rcr    ax,1
     a83:	loop   0xa7f
     a85:	mov    bx,word ptr [di]
     a87:	xor    cx,cx
     a89:	call   0xa8a	a8a: R_386_PC16	__LMUL
     a8c:	mov    word ptr [bp-42],dx
     a8f:	mov    word ptr [bp-44],ax
     a92:	cmp    dx,word ptr [bp-54]
     a95:	jne    0xa9a
     a97:	cmp    ax,word ptr [bp-56]
     a9a:	jbe    0xad4
     a9c:	mov    dx,word ptr [bp-42]
     a9f:	mov    ax,word ptr [bp-44]
     aa2:	mov    word ptr [bp-58],dx
     aa5:	mov    word ptr [bp-60],ax
     aa8:	mov    ax,0x800
     aab:	sub    ax,word ptr [di]
     aad:	mov    cx,0x5
     ab0:	shr    ax,cl
     ab2:	add    word ptr [di],ax
     ab4:	mov    ax,word ptr [bp-40]
     ab7:	shl    ax,1
     ab9:	shl    ax,1
     abb:	shl    ax,1
     abd:	shl    ax,1
     abf:	add    ax,di
     ac1:	add    ax,0x4
     ac4:	mov    word ptr [bp-20],ax
     ac7:	mov    word ptr [bp-22],0x0
     acc:	mov    word ptr [bp-24],0x3
     ad1:	jmp    0xbe8
     ad4:	sub    word ptr [bp-60],ax
     ad7:	sbb    word ptr [bp-58],dx
     ada:	sub    word ptr [bp-56],ax
     add:	sbb    word ptr [bp-54],dx
     ae0:	mov    ax,word ptr [di]
     ae2:	mov    cx,0x5
     ae5:	shr    ax,cl
     ae7:	sub    word ptr [di],ax
     ae9:	mov    ax,di
     aeb:	add    ax,0x2
     aee:	mov    word ptr [bp-20],ax
     af1:	cmp    word ptr [bp-58],0x100
     af6:	jne    0xafc
     af8:	cmp    word ptr [bp-60],0x0
     afc:	jae    0xb57
     afe:	mov    dx,word ptr [bp-66]
     b01:	mov    ax,word ptr [bp-68]
     b04:	cmp    dx,word ptr [bp-62]
     b07:	jne    0xb0c
     b09:	cmp    ax,word ptr [bp-64]
     b0c:	jne    0xb11
     b0e:	jmp    0x10ee
     b11:	mov    cl,0x8
     b13:	shl    word ptr [bp-60],1
     b16:	rcl    word ptr [bp-58],1
     b19:	loop   0xb13
     b1b:	mov    dx,word ptr [bp-54]
     b1e:	mov    ax,word ptr [bp-56]
     b21:	mov    cl,0x8
     b23:	shl    ax,1
     b25:	rcl    dx,1
     b27:	loop   0xb23
     b29:	push   ax
     b2a:	push   dx
     b2b:	les    bx,dword ptr [bp-68]
     b2e:	xor    si,si
     b30:	add    word ptr [bp-68],0x1
     b34:	adc    si,0x0
     b37:	mov    cx,0x0	b38: R_386_16	__AHSHIFT
     b3a:	shl    si,cl
     b3c:	add    word ptr [bp-66],si
     b3f:	mov    al,byte ptr es:[bx]
     b42:	xor    ah,ah
     b44:	test   ax,ax
     b46:	cwd
     b47:	mov    cx,dx
     b49:	mov    bx,ax
     b4b:	pop    dx
     b4c:	pop    ax
     b4d:	or     ax,bx
     b4f:	or     dx,cx
     b51:	mov    word ptr [bp-54],dx
     b54:	mov    word ptr [bp-56],ax
     b57:	mov    dx,word ptr [bp-58]
     b5a:	mov    ax,word ptr [bp-60]
     b5d:	mov    cx,0xb
     b60:	shr    dx,1
     b62:	rcr    ax,1
     b64:	loop   0xb60
     b66:	mov    bx,word ptr [bp-20]
     b69:	mov    bx,word ptr [bx]
     b6b:	mov    word ptr [bp-124],bx
     b6e:	xor    cx,cx
     b70:	call   0xb71	b71: R_386_PC16	__LMUL
     b73:	mov    word ptr [bp-42],dx
     b76:	mov    word ptr [bp-44],ax
     b79:	cmp    dx,word ptr [bp-54]
     b7c:	jne    0xb81
     b7e:	cmp    ax,word ptr [bp-56]
     b81:	jbe    0xbbd
     b83:	mov    dx,word ptr [bp-42]
     b86:	mov    ax,word ptr [bp-44]
     b89:	mov    word ptr [bp-58],dx
     b8c:	mov    word ptr [bp-60],ax
     b8f:	mov    ax,0x800
     b92:	mov    bx,word ptr [bp-20]
     b95:	sub    ax,word ptr [bx]
     b97:	mov    cx,0x5
     b9a:	shr    ax,cl
     b9c:	add    word ptr [bx],ax
     b9e:	mov    ax,word ptr [bp-40]
     ba1:	shl    ax,1
     ba3:	shl    ax,1
     ba5:	shl    ax,1
     ba7:	shl    ax,1
     ba9:	add    ax,di
     bab:	add    ax,0x104
     bae:	mov    word ptr [bp-20],ax
     bb1:	mov    word ptr [bp-22],0x8
     bb6:	mov    word ptr [bp-24],0x3
     bbb:	jmp    0xbe8
     bbd:	sub    word ptr [bp-60],ax
     bc0:	sbb    word ptr [bp-58],dx
     bc3:	sub    word ptr [bp-56],ax
     bc6:	sbb    word ptr [bp-54],dx
     bc9:	mov    ax,word ptr [bp-124]
     bcc:	mov    cx,0x5
     bcf:	shr    ax,cl
     bd1:	mov    bx,word ptr [bp-20]
     bd4:	sub    word ptr [bx],ax
     bd6:	mov    ax,di
     bd8:	add    ax,0x204
     bdb:	mov    word ptr [bp-20],ax
     bde:	mov    word ptr [bp-22],0x10
     be3:	mov    word ptr [bp-24],0x8
     be8:	mov    ax,word ptr [bp-24]
     beb:	mov    word ptr [bp-18],ax
     bee:	mov    di,0x1
     bf1:	mov    si,di
     bf3:	shl    si,1
     bf5:	add    si,word ptr [bp-20]
     bf8:	cmp    word ptr [bp-58],0x100
     bfd:	jne    0xc03
     bff:	cmp    word ptr [bp-60],0x0
     c03:	jae    0xc65
     c05:	mov    dx,word ptr [bp-66]
     c08:	mov    ax,word ptr [bp-68]
     c0b:	cmp    dx,word ptr [bp-62]
     c0e:	jne    0xc13
     c10:	cmp    ax,word ptr [bp-64]
     c13:	jne    0xc18
     c15:	jmp    0x10ee
     c18:	mov    word ptr [bp-16],si
     c1b:	mov    cx,0x8
     c1e:	shl    word ptr [bp-60],1
     c21:	rcl    word ptr [bp-58],1
     c24:	loop   0xc1e
     c26:	mov    dx,word ptr [bp-54]
     c29:	mov    ax,word ptr [bp-56]
     c2c:	mov    cl,0x8
     c2e:	shl    ax,1
     c30:	rcl    dx,1
     c32:	loop   0xc2e
     c34:	push   ax
     c35:	push   dx
     c36:	les    bx,dword ptr [bp-68]
     c39:	xor    si,si
     c3b:	add    word ptr [bp-68],0x1
     c3f:	adc    si,0x0
     c42:	mov    cx,0x0	c43: R_386_16	__AHSHIFT
     c45:	shl    si,cl
     c47:	add    word ptr [bp-66],si
     c4a:	mov    al,byte ptr es:[bx]
     c4d:	xor    ah,ah
     c4f:	test   ax,ax
     c51:	cwd
     c52:	mov    cx,dx
     c54:	mov    bx,ax
     c56:	pop    dx
     c57:	pop    ax
     c58:	or     ax,bx
     c5a:	or     dx,cx
     c5c:	mov    word ptr [bp-54],dx
     c5f:	mov    word ptr [bp-56],ax
     c62:	mov    si,word ptr [bp-16]
     c65:	mov    dx,word ptr [bp-58]
     c68:	mov    ax,word ptr [bp-60]
     c6b:	mov    cx,0xb
     c6e:	shr    dx,1
     c70:	rcr    ax,1
     c72:	loop   0xc6e
     c74:	mov    bx,word ptr [si]
     c76:	mov    word ptr [bp-122],bx
     c79:	xor    cx,cx
     c7b:	call   0xc7c	c7c: R_386_PC16	__LMUL
     c7e:	mov    word ptr [bp-42],dx
     c81:	mov    word ptr [bp-44],ax
     c84:	cmp    dx,word ptr [bp-54]
     c87:	jne    0xc8c
     c89:	cmp    ax,word ptr [bp-56]
     c8c:	jbe    0xcaa
     c8e:	mov    dx,word ptr [bp-42]
     c91:	mov    ax,word ptr [bp-44]
     c94:	mov    word ptr [bp-58],dx
     c97:	mov    word ptr [bp-60],ax
     c9a:	mov    dx,0x800
     c9d:	sub    dx,word ptr [si]
     c9f:	mov    cx,0x5
     ca2:	shr    dx,cl
     ca4:	add    word ptr [si],dx
     ca6:	shl    di,1
     ca8:	jmp    0xcc3
     caa:	sub    word ptr [bp-60],ax
     cad:	sbb    word ptr [bp-58],dx
     cb0:	sub    word ptr [bp-56],ax
     cb3:	sbb    word ptr [bp-54],dx
     cb6:	mov    dx,word ptr [bp-122]
     cb9:	mov    cx,0x5
     cbc:	shr    dx,cl
     cbe:	sub    word ptr [si],dx
     cc0:	shl    di,1
     cc2:	inc    di
     cc3:	dec    word ptr [bp-18]
     cc6:	je     0xccb
     cc8:	jmp    0xbf1
     ccb:	mov    word ptr [bp-72],di
     cce:	mov    ax,0x1
     cd1:	mov    cx,word ptr [bp-24]
     cd4:	shl    ax,cl
     cd6:	sub    word ptr [bp-72],ax
     cd9:	mov    ax,word ptr [bp-22]
     cdc:	add    word ptr [bp-72],ax
     cdf:	cmp    word ptr [bp-90],0x4
     ce3:	jl     0xce8
     ce5:	jmp    0x1016
     ce8:	add    word ptr [bp-90],0x7
     cec:	cmp    word ptr [bp-72],0x4
     cf0:	jge    0xcf7
     cf2:	mov    di,word ptr [bp-72]
     cf5:	jmp    0xcfa
     cf7:	mov    di,0x3
     cfa:	mov    cx,0x7
     cfd:	shl    di,cl
     cff:	add    di,word ptr [bp-112]
     d02:	add    di,0x360
     d06:	mov    word ptr [bp-12],0x6
     d0b:	mov    si,0x1
     d0e:	mov    cx,si
     d10:	shl    cx,1
     d12:	add    cx,di
     d14:	mov    word ptr [bp-10],cx
     d17:	cmp    word ptr [bp-58],0x100
     d1c:	jne    0xd22
     d1e:	cmp    word ptr [bp-60],0x0
     d22:	jae    0xd84
     d24:	mov    dx,word ptr [bp-66]
     d27:	mov    ax,word ptr [bp-68]
     d2a:	cmp    dx,word ptr [bp-62]
     d2d:	jne    0xd32
     d2f:	cmp    ax,word ptr [bp-64]
     d32:	jne    0xd37
     d34:	jmp    0x10ee
     d37:	mov    word ptr [bp-14],si
     d3a:	mov    cx,0x8
     d3d:	shl    word ptr [bp-60],1
     d40:	rcl    word ptr [bp-58],1
     d43:	loop   0xd3d
     d45:	mov    dx,word ptr [bp-54]
     d48:	mov    ax,word ptr [bp-56]
     d4b:	mov    cl,0x8
     d4d:	shl    ax,1
     d4f:	rcl    dx,1
     d51:	loop   0xd4d
     d53:	push   ax
     d54:	push   dx
     d55:	les    bx,dword ptr [bp-68]
     d58:	xor    si,si
     d5a:	add    word ptr [bp-68],0x1
     d5e:	adc    si,0x0
     d61:	mov    cx,0x0	d62: R_386_16	__AHSHIFT
     d64:	shl    si,cl
     d66:	add    word ptr [bp-66],si
     d69:	mov    al,byte ptr es:[bx]
     d6c:	xor    ah,ah
     d6e:	test   ax,ax
     d70:	cwd
     d71:	mov    cx,dx
     d73:	mov    bx,ax
     d75:	pop    dx
     d76:	pop    ax
     d77:	or     ax,bx
     d79:	or     dx,cx
     d7b:	mov    word ptr [bp-54],dx
     d7e:	mov    word ptr [bp-56],ax
     d81:	mov    si,word ptr [bp-14]
     d84:	mov    dx,word ptr [bp-58]
     d87:	mov    ax,word ptr [bp-60]
     d8a:	mov    cx,0xb
     d8d:	shr    dx,1
     d8f:	rcr    ax,1
     d91:	loop   0xd8d
     d93:	mov    bx,word ptr [bp-10]
     d96:	mov    bx,word ptr [bx]
     d98:	mov    word ptr [bp-120],bx
     d9b:	xor    cx,cx
     d9d:	call   0xd9e	d9e: R_386_PC16	__LMUL
     da0:	mov    word ptr [bp-42],dx
     da3:	mov    word ptr [bp-44],ax
     da6:	cmp    dx,word ptr [bp-54]
     da9:	jne    0xdae
     dab:	cmp    ax,word ptr [bp-56]
     dae:	jbe    0xdcf
     db0:	mov    dx,word ptr [bp-42]
     db3:	mov    ax,word ptr [bp-44]
     db6:	mov    word ptr [bp-58],dx
     db9:	mov    word ptr [bp-60],ax
     dbc:	mov    dx,0x800
     dbf:	mov    bx,word ptr [bp-10]
     dc2:	sub    dx,word ptr [bx]
     dc4:	mov    cx,0x5
     dc7:	shr    dx,cl
     dc9:	add    word ptr [bx],dx
     dcb:	shl    si,1
     dcd:	jmp    0xdeb
     dcf:	sub    word ptr [bp-60],ax
     dd2:	sbb    word ptr [bp-58],dx
     dd5:	sub    word ptr [bp-56],ax
     dd8:	sbb    word ptr [bp-54],dx
     ddb:	mov    dx,word ptr [bp-120]
     dde:	mov    cx,0x5
     de1:	shr    dx,cl
     de3:	mov    bx,word ptr [bp-10]
     de6:	sub    word ptr [bx],dx
     de8:	shl    si,1
     dea:	inc    si
     deb:	dec    word ptr [bp-12]
     dee:	je     0xdf3
     df0:	jmp    0xd0e
     df3:	mov    di,word ptr [bp-72]
     df6:	add    si,0xffffffc0
     df9:	cmp    si,0x4
     dfc:	jge    0xe01
     dfe:	jmp    0xff3
     e01:	mov    cx,si
     e03:	sar    cx,1
     e05:	dec    cx
     e06:	mov    word ptr [bp-8],cx
     e09:	mov    ax,si
     e0b:	and    ax,0x1
     e0e:	or     al,0x2
     e10:	xor    cx,cx
     e12:	mov    word ptr [bp-86],cx
     e15:	mov    word ptr [bp-88],ax
     e18:	cmp    si,0xe
     e1b:	jge    0xe42
     e1d:	mov    word ptr [bp-72],di
     e20:	mov    cx,word ptr [bp-8]
     e23:	jcxz   0xe2d
     e25:	shl    word ptr [bp-88],1
     e28:	rcl    word ptr [bp-86],1
     e2b:	loop   0xe25
     e2d:	mov    di,word ptr [bp-88]
     e30:	shl    di,1
     e32:	add    di,word ptr [bp-112]
     e35:	add    di,0x55e
     e39:	mov    cx,si
     e3b:	shl    cx,1
     e3d:	sub    di,cx
     e3f:	jmp    0xef5
     e42:	add    word ptr [bp-8],0xfffffffc
     e46:	cmp    word ptr [bp-58],0x100
     e4b:	jne    0xe51
     e4d:	cmp    word ptr [bp-60],0x0
     e51:	jae    0xead
     e53:	mov    dx,word ptr [bp-66]
     e56:	mov    ax,word ptr [bp-68]
     e59:	cmp    dx,word ptr [bp-62]
     e5c:	jne    0xe61
     e5e:	cmp    ax,word ptr [bp-64]
     e61:	jne    0xe66
     e63:	jmp    0x10ee
     e66:	mov    cx,0x8
     e69:	shl    word ptr [bp-60],1
     e6c:	rcl    word ptr [bp-58],1
     e6f:	loop   0xe69
     e71:	mov    dx,word ptr [bp-54]
     e74:	mov    ax,word ptr [bp-56]
     e77:	mov    cl,0x8
     e79:	shl    ax,1
     e7b:	rcl    dx,1
     e7d:	loop   0xe79
     e7f:	push   ax
     e80:	push   dx
     e81:	les    bx,dword ptr [bp-68]
     e84:	xor    si,si
     e86:	add    word ptr [bp-68],0x1
     e8a:	adc    si,0x0
     e8d:	mov    cx,0x0	e8e: R_386_16	__AHSHIFT
     e90:	shl    si,cl
     e92:	add    word ptr [bp-66],si
     e95:	mov    al,byte ptr es:[bx]
     e98:	xor    ah,ah
     e9a:	test   ax,ax
     e9c:	cwd
     e9d:	mov    cx,dx
     e9f:	mov    bx,ax
     ea1:	pop    dx
     ea2:	pop    ax
     ea3:	or     ax,bx
     ea5:	or     dx,cx
     ea7:	mov    word ptr [bp-54],dx
     eaa:	mov    word ptr [bp-56],ax
     ead:	shr    word ptr [bp-58],1
     eb0:	rcr    word ptr [bp-60],1
     eb3:	mov    dx,word ptr [bp-58]
     eb6:	mov    ax,word ptr [bp-60]
     eb9:	shl    word ptr [bp-88],1
     ebc:	rcl    word ptr [bp-86],1
     ebf:	cmp    word ptr [bp-54],dx
     ec2:	jne    0xec7
     ec4:	cmp    word ptr [bp-56],ax
     ec7:	jb     0xed3
     ec9:	sub    word ptr [bp-56],ax
     ecc:	sbb    word ptr [bp-54],dx
     ecf:	or     byte ptr [bp-88],0x1
     ed3:	dec    word ptr [bp-8]
     ed6:	je     0xedb
     ed8:	jmp    0xe46
     edb:	mov    word ptr [bp-72],di
     ede:	mov    di,word ptr [bp-112]
     ee1:	add    di,0x644
     ee5:	mov    cx,0x4
     ee8:	shl    word ptr [bp-88],1
     eeb:	rcl    word ptr [bp-86],1
     eee:	loop   0xee8
     ef0:	mov    word ptr [bp-8],0x4
     ef5:	mov    word ptr [bp-6],0x1
     efa:	mov    si,0x1
     efd:	mov    cx,si
     eff:	shl    cx,1
     f01:	add    cx,di
     f03:	mov    word ptr [bp-2],cx
     f06:	cmp    word ptr [bp-58],0x100
     f0b:	jne    0xf11
     f0d:	cmp    word ptr [bp-60],0x0
     f11:	jae    0xf73
     f13:	mov    dx,word ptr [bp-66]
     f16:	mov    ax,word ptr [bp-68]
     f19:	cmp    dx,word ptr [bp-62]
     f1c:	jne    0xf21
     f1e:	cmp    ax,word ptr [bp-64]
     f21:	jne    0xf26
     f23:	jmp    0x10ee
     f26:	mov    word ptr [bp-4],si
     f29:	mov    cx,0x8
     f2c:	shl    word ptr [bp-60],1
     f2f:	rcl    word ptr [bp-58],1
     f32:	loop   0xf2c
     f34:	mov    dx,word ptr [bp-54]
     f37:	mov    ax,word ptr [bp-56]
     f3a:	mov    cl,0x8
     f3c:	shl    ax,1
     f3e:	rcl    dx,1
     f40:	loop   0xf3c
     f42:	push   ax
     f43:	push   dx
     f44:	les    bx,dword ptr [bp-68]
     f47:	xor    si,si
     f49:	add    word ptr [bp-68],0x1
     f4d:	adc    si,0x0
     f50:	mov    cx,0x0	f51: R_386_16	__AHSHIFT
     f53:	shl    si,cl
     f55:	add    word ptr [bp-66],si
     f58:	mov    al,byte ptr es:[bx]
     f5b:	xor    ah,ah
     f5d:	test   ax,ax
     f5f:	cwd
     f60:	mov    cx,dx
     f62:	mov    bx,ax
     f64:	pop    dx
     f65:	pop    ax
     f66:	or     ax,bx
     f68:	or     dx,cx
     f6a:	mov    word ptr [bp-54],dx
     f6d:	mov    word ptr [bp-56],ax
     f70:	mov    si,word ptr [bp-4]
     f73:	mov    dx,word ptr [bp-58]
     f76:	mov    ax,word ptr [bp-60]
     f79:	mov    cx,0xb
     f7c:	shr    dx,1
     f7e:	rcr    ax,1
     f80:	loop   0xf7c
     f82:	mov    bx,word ptr [bp-2]
     f85:	mov    bx,word ptr [bx]
     f87:	mov    word ptr [bp-118],bx
     f8a:	xor    cx,cx
     f8c:	call   0xf8d	f8d: R_386_PC16	__LMUL
     f8f:	mov    word ptr [bp-42],dx
     f92:	mov    word ptr [bp-44],ax
     f95:	cmp    dx,word ptr [bp-54]
     f98:	jne    0xf9d
     f9a:	cmp    ax,word ptr [bp-56]
     f9d:	jbe    0xfbe
     f9f:	mov    dx,word ptr [bp-42]
     fa2:	mov    ax,word ptr [bp-44]
     fa5:	mov    word ptr [bp-58],dx
     fa8:	mov    word ptr [bp-60],ax
     fab:	mov    dx,0x800
     fae:	mov    bx,word ptr [bp-2]
     fb1:	sub    dx,word ptr [bx]
     fb3:	mov    cx,0x5
     fb6:	shr    dx,cl
     fb8:	add    word ptr [bx],dx
     fba:	shl    si,1
     fbc:	jmp    0xfe6
     fbe:	sub    word ptr [bp-60],ax
     fc1:	sbb    word ptr [bp-58],dx
     fc4:	sub    word ptr [bp-56],ax
     fc7:	sbb    word ptr [bp-54],dx
     fca:	mov    dx,word ptr [bp-118]
     fcd:	mov    cx,0x5
     fd0:	shr    dx,cl
     fd2:	mov    bx,word ptr [bp-2]
     fd5:	sub    word ptr [bx],dx
     fd7:	shl    si,1
     fd9:	inc    si
     fda:	mov    ax,word ptr [bp-6]
     fdd:	test   ax,ax
     fdf:	cwd
     fe0:	or     word ptr [bp-88],ax
     fe3:	or     word ptr [bp-86],dx
     fe6:	shl    word ptr [bp-6],1
     fe9:	dec    word ptr [bp-8]
     fec:	je     0xff1
     fee:	jmp    0xefd
     ff1:	jmp    0x1001
     ff3:	mov    ax,si
     ff5:	test   ax,ax
     ff7:	cwd
     ff8:	mov    word ptr [bp-86],dx
     ffb:	mov    word ptr [bp-88],ax
     ffe:	mov    word ptr [bp-72],di
    1001:	add    word ptr [bp-88],0x1
    1005:	adc    word ptr [bp-86],0x0
    1009:	mov    dx,word ptr [bp-86]
    100c:	mov    ax,word ptr [bp-88]
    100f:	or     dx,ax
    1011:	jne    0x1016
    1013:	jmp    0x10d1
    1016:	add    word ptr [bp-72],0x2
    101a:	mov    dx,word ptr [bp-86]
    101d:	mov    ax,word ptr [bp-88]
    1020:	cmp    dx,word ptr [bp-106]
    1023:	jne    0x1028
    1025:	cmp    ax,word ptr [bp-108]
    1028:	jbe    0x102d
    102a:	jmp    0x10ee
    102d:	mov    di,word ptr [bp-72]
    1030:	mov    cx,word ptr [bp+18]
    1033:	mov    bx,word ptr [bp+16]
    1036:	mov    dx,word ptr [bp-106]
    1039:	mov    si,word ptr [bp-108]
    103c:	add    si,bx
    103e:	mov    bx,cx
    1040:	adc    dx,0x0
    1043:	mov    cx,0x0	1044: R_386_16	__AHSHIFT
    1046:	shl    dx,cl
    1048:	add    dx,bx
    104a:	mov    cx,dx
    104c:	mov    bx,si
    104e:	mov    dx,word ptr [bp-86]
    1051:	mov    si,ax
    1053:	neg    dx
    1055:	neg    si
    1057:	sbb    dx,0x0
    105a:	add    si,bx
    105c:	mov    bx,cx
    105e:	adc    dx,0x0
    1061:	mov    cx,0x0	1062: R_386_16	__AHSHIFT
    1064:	shl    dx,cl
    1066:	add    dx,bx
    1068:	mov    word ptr [bp-0x8c],dx
    106c:	mov    word ptr [bp-0x8e],si
    1070:	les    bx,dword ptr [bp-0x8e]
    1074:	mov    al,byte ptr es:[bx]
    1077:	mov    byte ptr [bp-104],al
    107a:	dec    di
    107b:	mov    bx,0x1
    107e:	xor    dx,dx
    1080:	add    word ptr [bp-0x8e],bx
    1084:	adc    dx,0x0
    1087:	mov    cx,0x0	1088: R_386_16	__AHSHIFT
    108a:	shl    dx,cl
    108c:	add    word ptr [bp-0x8c],dx
    1090:	mov    bx,word ptr [bp-108]
    1093:	mov    dx,word ptr [bp-106]
    1096:	add    word ptr [bp-108],0x1
    109a:	adc    word ptr [bp-106],0x0
    109e:	mov    cx,word ptr [bp+18]
    10a1:	mov    si,word ptr [bp+16]
    10a4:	add    bx,si
    10a6:	mov    si,cx
    10a8:	adc    dx,0x0
    10ab:	mov    cx,0x0	10ac: R_386_16	__AHSHIFT
    10ae:	shl    dx,cl
    10b0:	add    dx,si
    10b2:	movl   es,dx
    10b4:	mov    byte ptr es:[bx],al
    10b7:	test   di,di
    10b9:	jne    0x10be
    10bb:	jmp    0x4d8
    10be:	mov    dx,word ptr [bp-106]
    10c1:	mov    ax,word ptr [bp-108]
    10c4:	cmp    dx,word ptr [bp+22]
    10c7:	jne    0x10cc
    10c9:	cmp    ax,word ptr [bp+20]
    10cc:	jb     0x1070
    10ce:	jmp    0x4d8
    10d1:	cmp    word ptr [bp-58],0x100
    10d6:	jne    0x10dc
    10d8:	cmp    word ptr [bp-60],0x0
    10dc:	jae    0x1109
    10de:	mov    dx,word ptr [bp-66]
    10e1:	mov    ax,word ptr [bp-68]
    10e4:	cmp    dx,word ptr [bp-62]
    10e7:	jne    0x10f7
    10e9:	cmp    ax,word ptr [bp-64]
    10ec:	jne    0x10f7
    10ee:	mov    ax,0x1
    10f1:	pop    di
    10f2:	pop    si
    10f3:	mov    sp,bp
    10f5:	pop    bp
    10f6:	ret
    10f7:	mov    ax,0x1
    10fa:	cwd
    10fb:	add    word ptr [bp-68],ax
    10fe:	adc    dx,0x0
    1101:	mov    cx,0x0	1102: R_386_16	__AHSHIFT
    1104:	shl    dx,cl
    1106:	add    word ptr [bp-66],dx
    1109:	push   word ptr [bp+8]
    110c:	push   word ptr [bp+6]
    110f:	push   word ptr [bp-66]
    1112:	push   word ptr [bp-68]
    1115:	call   0x1116	1116: R_386_PC16	__aNahdiff
    1118:	mov    bx,word ptr [bp+14]
    111b:	mov    word ptr [bx+2],dx
    111e:	mov    word ptr [bx],ax
    1120:	mov    dx,word ptr [bp-106]
    1123:	mov    ax,word ptr [bp-108]
    1126:	mov    bx,word ptr [bp+24]
    1129:	mov    word ptr [bx+2],dx
    112c:	mov    word ptr [bx],ax
    112e:	xor    ax,ax
    1130:	pop    di
    1131:	pop    si
    1132:	mov    sp,bp
    1134:	pop    bp
    1135:	ret
