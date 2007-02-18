
tmp/lzma_d_cs.o:     file format elf32-i386

Disassembly of section .text.LzmaDecode:

00000000 <.text.LzmaDecode>:
       0:	push   si
       1:	push   di
       2:	push   bp
       3:	mov    bp,sp
       5:	sub    sp,0x66
       8:	mov    si,word ptr [bp+8]
       b:	lea    bx,[si+4]
       e:	mov    word ptr [bp-34],bx
      11:	xor    ax,ax
      13:	mov    word ptr [bp-20],ax
      16:	mov    word ptr [bp-18],ax
      19:	mov    byte ptr [bp-2],0x0
      1d:	mov    cl,byte ptr [si+2]
      20:	mov    ax,0x1
      23:	shl    ax,cl
      25:	dec    ax
      26:	mov    word ptr [bp-88],ax
      29:	mov    cl,byte ptr [si+1]
      2c:	mov    ax,0x1
      2f:	shl    ax,cl
      31:	dec    ax
      32:	cwd
      33:	mov    word ptr [bp-86],ax
      36:	mov    word ptr [bp-84],dx
      39:	mov    al,byte ptr [si]
      3b:	xor    ah,ah
      3d:	mov    word ptr [bp-78],ax
      40:	mov    word ptr [bp-16],0x0
      45:	mov    word ptr [bp-102],0x1
      4a:	mov    word ptr [bp-100],0x0
      4f:	mov    word ptr [bp-52],0x1
      54:	mov    word ptr [bp-50],0x0
      59:	mov    word ptr [bp-56],0x1
      5e:	mov    word ptr [bp-54],0x0
      63:	mov    word ptr [bp-68],0x1
      68:	mov    word ptr [bp-74],0x0
      6d:	mov    bx,word ptr [bp+18]
      70:	mov    word ptr [bx],0x0
      74:	mov    word ptr [bx+2],0x0
      79:	mov    bx,word ptr [bp+28]
      7c:	mov    word ptr [bx],0x0
      80:	mov    word ptr [bx+2],0x0
      85:	mov    dl,byte ptr [si+1]
      88:	xor    dh,dh
      8a:	mov    cx,ax
      8c:	add    cx,dx
      8e:	mov    ax,0x300
      91:	xor    dl,dl
      93:	jcxz   0x9b
      95:	shl    ax,1
      97:	rcl    dx,1
      99:	loop   0x95
      9b:	mov    si,ax
      9d:	add    si,0x736
      a1:	mov    cx,dx
      a3:	adc    cx,0x0
      a6:	xor    dx,dx
      a8:	xor    ax,ax
      aa:	cmp    ax,cx
      ac:	jb     0xb4
      ae:	jne    0xc7
      b0:	cmp    dx,si
      b2:	jae    0xc7
      b4:	mov    bx,dx
      b6:	shl    bx,1
      b8:	add    bx,word ptr [bp-34]
      bb:	mov    word ptr [bx],0x400
      bf:	add    dx,0x1
      c2:	adc    ax,0x0
      c5:	jmp    0xaa
      c7:	mov    bx,word ptr [bp+10]
      ca:	mov    word ptr [bp-12],bx
      cd:	mov    ax,word ptr [bp+12]
      d0:	mov    word ptr [bp-10],ax
      d3:	mov    ax,bx
      d5:	mov    dx,word ptr [bp+12]
      d8:	mov    bx,word ptr [bp+14]
      db:	mov    cx,word ptr [bp+16]
      de:	call   0xdf	df: R_386_PC16	__PIA
      e1:	mov    word ptr [bp-30],ax
      e4:	mov    word ptr [bp-28],dx
      e7:	xor    ax,ax
      e9:	mov    word ptr [bp-6],ax
      ec:	mov    word ptr [bp-4],ax
      ef:	mov    si,0xffff
      f2:	mov    di,si
      f4:	mov    word ptr [bp-72],ax
      f7:	jmp    0x13c
      f9:	les    bx,dword ptr [bp-12]
      fc:	mov    al,byte ptr es:[bx]
      ff:	mov    byte ptr [bp-90],al
     102:	mov    byte ptr [bp-89],0x0
     106:	mov    ax,word ptr [bp-6]
     109:	mov    dx,word ptr [bp-4]
     10c:	mov    cx,0x8
     10f:	shl    ax,1
     111:	rcl    dx,1
     113:	loop   0x10f
     115:	mov    bx,word ptr [bp-90]
     118:	or     bx,ax
     11a:	mov    word ptr [bp-6],bx
     11d:	mov    word ptr [bp-4],dx
     120:	mov    ax,word ptr [bp-12]
     123:	movl   dx,es
     125:	mov    bx,0x1
     128:	xor    cx,cx
     12a:	call   0x12b	12b: R_386_PC16	__PIA
     12d:	mov    word ptr [bp-12],ax
     130:	mov    word ptr [bp-10],dx
     133:	inc    word ptr [bp-72]
     136:	cmp    word ptr [bp-72],0x5
     13a:	jge    0x153
     13c:	mov    ax,word ptr [bp-12]
     13f:	mov    dx,word ptr [bp-10]
     142:	mov    bx,word ptr [bp-30]
     145:	mov    cx,word ptr [bp-28]
     148:	call   0x149	149: R_386_PC16	__PTC
     14b:	jne    0xf9
     14d:	mov    ax,0x1
     150:	jmp    0x10f3
     153:	mov    ax,word ptr [bp-18]
     156:	cmp    ax,word ptr [bp+26]
     159:	jb     0x168
     15b:	je     0x160
     15d:	jmp    0x109e
     160:	mov    ax,word ptr [bp-20]
     163:	cmp    ax,word ptr [bp+24]
     166:	jae    0x15d
     168:	mov    ax,word ptr [bp-20]
     16b:	and    ax,word ptr [bp-88]
     16e:	mov    word ptr [bp-58],ax
     171:	mov    cl,0x5
     173:	mov    dx,word ptr [bp-16]
     176:	shl    dx,cl
     178:	add    dx,word ptr [bp-34]
     17b:	shl    ax,1
     17d:	add    dx,ax
     17f:	mov    word ptr [bp-14],dx
     182:	cmp    di,0x100
     186:	jae    0x1dc
     188:	mov    ax,word ptr [bp-12]
     18b:	mov    dx,word ptr [bp-10]
     18e:	mov    bx,word ptr [bp-30]
     191:	mov    cx,word ptr [bp-28]
     194:	call   0x195	195: R_386_PC16	__PTC
     197:	je     0x14d
     199:	mov    cx,0x8
     19c:	shl    si,1
     19e:	rcl    di,1
     1a0:	loop   0x19c
     1a2:	les    bx,dword ptr [bp-12]
     1a5:	mov    al,byte ptr es:[bx]
     1a8:	mov    byte ptr [bp-90],al
     1ab:	mov    byte ptr [bp-89],0x0
     1af:	mov    ax,word ptr [bp-6]
     1b2:	mov    dx,word ptr [bp-4]
     1b5:	mov    cx,0x8
     1b8:	shl    ax,1
     1ba:	rcl    dx,1
     1bc:	loop   0x1b8
     1be:	mov    bx,word ptr [bp-90]
     1c1:	or     bx,ax
     1c3:	mov    word ptr [bp-6],bx
     1c6:	mov    word ptr [bp-4],dx
     1c9:	mov    ax,word ptr [bp-12]
     1cc:	movl   dx,es
     1ce:	mov    bx,0x1
     1d1:	xor    cx,cx
     1d3:	call   0x1d4	1d4: R_386_PC16	__PIA
     1d6:	mov    word ptr [bp-12],ax
     1d9:	mov    word ptr [bp-10],dx
     1dc:	mov    word ptr [bp-98],si
     1df:	mov    word ptr [bp-96],di
     1e2:	mov    cx,0xb
     1e5:	shr    word ptr [bp-96],1
     1e8:	rcr    word ptr [bp-98],1
     1eb:	loop   0x1e5
     1ed:	mov    bx,word ptr [bp-14]
     1f0:	mov    bx,word ptr [bx]
     1f2:	mov    ax,word ptr [bp-98]
     1f5:	mov    dx,word ptr [bp-96]
     1f8:	xor    cx,cx
     1fa:	call   0x1fb	1fb: R_386_PC16	__U4M
     1fd:	mov    word ptr [bp-8],ax
     200:	mov    word ptr [bp-94],dx
     203:	mov    ax,word ptr [bp-4]
     206:	cmp    ax,dx
     208:	jb     0x217
     20a:	je     0x20f
     20c:	jmp    0x4d9
     20f:	mov    ax,word ptr [bp-6]
     212:	cmp    ax,word ptr [bp-8]
     215:	jae    0x20c
     217:	mov    word ptr [bp-26],0x1
     21c:	mov    si,word ptr [bp-8]
     21f:	mov    di,dx
     221:	mov    ax,0x800
     224:	mov    bx,word ptr [bp-14]
     227:	sub    ax,word ptr [bx]
     229:	mov    cl,0x5
     22b:	shr    ax,cl
     22d:	add    word ptr [bx],ax
     22f:	mov    cx,0x8
     232:	sub    cx,word ptr [bp-78]
     235:	mov    al,byte ptr [bp-2]
     238:	xor    ah,ah
     23a:	sar    ax,cl
     23c:	cwd
     23d:	mov    word ptr [bp-90],ax
     240:	mov    bx,dx
     242:	mov    ax,word ptr [bp-20]
     245:	and    ax,word ptr [bp-86]
     248:	mov    dx,word ptr [bp-18]
     24b:	and    dx,word ptr [bp-84]
     24e:	mov    cx,word ptr [bp-78]
     251:	jcxz   0x259
     253:	shl    ax,1
     255:	rcl    dx,1
     257:	loop   0x253
     259:	add    ax,word ptr [bp-90]
     25c:	adc    dx,bx
     25e:	mov    bx,0x300
     261:	xor    cx,cx
     263:	call   0x264	264: R_386_PC16	__U4M
     266:	shl    ax,1
     268:	rcl    dx,1
     26a:	mov    dx,word ptr [bp-34]
     26d:	add    dx,0xe6c
     271:	add    dx,ax
     273:	mov    word ptr [bp-14],dx
     276:	cmp    word ptr [bp-16],0x7
     27a:	jge    0x27f
     27c:	jmp    0x3a7
     27f:	mov    bx,word ptr [bp-20]
     282:	sub    bx,word ptr [bp-102]
     285:	mov    cx,word ptr [bp-18]
     288:	sbb    cx,word ptr [bp-100]
     28b:	mov    ax,word ptr [bp+20]
     28e:	mov    dx,word ptr [bp+22]
     291:	call   0x292	292: R_386_PC16	__PIA
     294:	mov    bx,ax
     296:	movl   es,dx
     298:	mov    al,byte ptr es:[bx]
     29b:	xor    ah,ah
     29d:	mov    word ptr [bp-76],ax
     2a0:	shl    word ptr [bp-76],1
     2a3:	mov    ax,word ptr [bp-76]
     2a6:	xor    al,al
     2a8:	and    ah,0x1
     2ab:	mov    word ptr [bp-66],ax
     2ae:	shl    ax,1
     2b0:	mov    dx,word ptr [bp-14]
     2b3:	add    dh,0x2
     2b6:	add    dx,ax
     2b8:	mov    ax,word ptr [bp-26]
     2bb:	shl    ax,1
     2bd:	add    dx,ax
     2bf:	mov    word ptr [bp-46],dx
     2c2:	cmp    di,0x100
     2c6:	jae    0x31f
     2c8:	mov    ax,word ptr [bp-12]
     2cb:	mov    dx,word ptr [bp-10]
     2ce:	mov    bx,word ptr [bp-30]
     2d1:	mov    cx,word ptr [bp-28]
     2d4:	call   0x2d5	2d5: R_386_PC16	__PTC
     2d7:	jne    0x2dc
     2d9:	jmp    0x14d
     2dc:	mov    cx,0x8
     2df:	shl    si,1
     2e1:	rcl    di,1
     2e3:	loop   0x2df
     2e5:	les    bx,dword ptr [bp-12]
     2e8:	mov    al,byte ptr es:[bx]
     2eb:	mov    byte ptr [bp-90],al
     2ee:	mov    byte ptr [bp-89],0x0
     2f2:	mov    ax,word ptr [bp-6]
     2f5:	mov    dx,word ptr [bp-4]
     2f8:	mov    cx,0x8
     2fb:	shl    ax,1
     2fd:	rcl    dx,1
     2ff:	loop   0x2fb
     301:	mov    bx,word ptr [bp-90]
     304:	or     bx,ax
     306:	mov    word ptr [bp-6],bx
     309:	mov    word ptr [bp-4],dx
     30c:	mov    ax,word ptr [bp-12]
     30f:	movl   dx,es
     311:	mov    bx,0x1
     314:	xor    cx,cx
     316:	call   0x317	317: R_386_PC16	__PIA
     319:	mov    word ptr [bp-12],ax
     31c:	mov    word ptr [bp-10],dx
     31f:	mov    word ptr [bp-98],si
     322:	mov    word ptr [bp-96],di
     325:	mov    cx,0xb
     328:	shr    word ptr [bp-96],1
     32b:	rcr    word ptr [bp-98],1
     32e:	loop   0x328
     330:	mov    bx,word ptr [bp-46]
     333:	mov    bx,word ptr [bx]
     335:	mov    ax,word ptr [bp-98]
     338:	mov    dx,word ptr [bp-96]
     33b:	xor    cx,cx
     33d:	call   0x33e	33e: R_386_PC16	__U4M
     340:	mov    word ptr [bp-8],ax
     343:	mov    word ptr [bp-94],dx
     346:	mov    ax,word ptr [bp-4]
     349:	cmp    ax,dx
     34b:	jb     0x357
     34d:	jne    0x375
     34f:	mov    ax,word ptr [bp-6]
     352:	cmp    ax,word ptr [bp-8]
     355:	jae    0x375
     357:	mov    si,word ptr [bp-8]
     35a:	mov    di,dx
     35c:	mov    ax,0x800
     35f:	mov    bx,word ptr [bp-46]
     362:	sub    ax,word ptr [bx]
     364:	mov    cl,0x5
     366:	shr    ax,cl
     368:	add    word ptr [bx],ax
     36a:	shl    word ptr [bp-26],1
     36d:	cmp    word ptr [bp-66],0x0
     371:	jne    0x3a7
     373:	jmp    0x39d
     375:	sub    si,word ptr [bp-8]
     378:	sbb    di,dx
     37a:	mov    ax,word ptr [bp-8]
     37d:	sub    word ptr [bp-6],ax
     380:	sbb    word ptr [bp-4],dx
     383:	mov    cl,0x5
     385:	mov    bx,word ptr [bp-46]
     388:	mov    ax,word ptr [bx]
     38a:	shr    ax,cl
     38c:	sub    word ptr [bx],ax
     38e:	mov    ax,word ptr [bp-26]
     391:	add    ax,ax
     393:	inc    ax
     394:	mov    word ptr [bp-26],ax
     397:	cmp    word ptr [bp-66],0x0
     39b:	je     0x3a7
     39d:	cmp    word ptr [bp-26],0x100
     3a2:	jge    0x3a7
     3a4:	jmp    0x2a0
     3a7:	mov    ax,word ptr [bp-26]
     3aa:	cmp    ax,0x100
     3ad:	jl     0x3b2
     3af:	jmp    0x48f
     3b2:	shl    ax,1
     3b4:	mov    bx,word ptr [bp-14]
     3b7:	add    bx,ax
     3b9:	mov    word ptr [bp-40],bx
     3bc:	cmp    di,0x100
     3c0:	jae    0x419
     3c2:	mov    ax,word ptr [bp-12]
     3c5:	mov    dx,word ptr [bp-10]
     3c8:	mov    bx,word ptr [bp-30]
     3cb:	mov    cx,word ptr [bp-28]
     3ce:	call   0x3cf	3cf: R_386_PC16	__PTC
     3d1:	jne    0x3d6
     3d3:	jmp    0x14d
     3d6:	mov    cx,0x8
     3d9:	shl    si,1
     3db:	rcl    di,1
     3dd:	loop   0x3d9
     3df:	les    bx,dword ptr [bp-12]
     3e2:	mov    al,byte ptr es:[bx]
     3e5:	mov    byte ptr [bp-90],al
     3e8:	mov    byte ptr [bp-89],0x0
     3ec:	mov    ax,word ptr [bp-6]
     3ef:	mov    dx,word ptr [bp-4]
     3f2:	mov    cx,0x8
     3f5:	shl    ax,1
     3f7:	rcl    dx,1
     3f9:	loop   0x3f5
     3fb:	mov    bx,word ptr [bp-90]
     3fe:	or     bx,ax
     400:	mov    word ptr [bp-6],bx
     403:	mov    word ptr [bp-4],dx
     406:	mov    ax,word ptr [bp-12]
     409:	movl   dx,es
     40b:	mov    bx,0x1
     40e:	xor    cx,cx
     410:	call   0x411	411: R_386_PC16	__PIA
     413:	mov    word ptr [bp-12],ax
     416:	mov    word ptr [bp-10],dx
     419:	mov    word ptr [bp-98],si
     41c:	mov    word ptr [bp-96],di
     41f:	mov    cx,0xb
     422:	shr    word ptr [bp-96],1
     425:	rcr    word ptr [bp-98],1
     428:	loop   0x422
     42a:	mov    bx,word ptr [bp-40]
     42d:	mov    bx,word ptr [bx]
     42f:	mov    ax,word ptr [bp-98]
     432:	mov    dx,word ptr [bp-96]
     435:	xor    cx,cx
     437:	call   0x438	438: R_386_PC16	__U4M
     43a:	mov    word ptr [bp-8],ax
     43d:	mov    word ptr [bp-94],dx
     440:	mov    ax,word ptr [bp-4]
     443:	cmp    ax,dx
     445:	jb     0x451
     447:	jne    0x46a
     449:	mov    ax,word ptr [bp-6]
     44c:	cmp    ax,word ptr [bp-8]
     44f:	jae    0x46a
     451:	mov    si,word ptr [bp-8]
     454:	mov    di,dx
     456:	mov    ax,0x800
     459:	mov    bx,word ptr [bp-40]
     45c:	sub    ax,word ptr [bx]
     45e:	mov    cl,0x5
     460:	shr    ax,cl
     462:	add    word ptr [bx],ax
     464:	shl    word ptr [bp-26],1
     467:	jmp    0x3a7
     46a:	sub    si,word ptr [bp-8]
     46d:	sbb    di,dx
     46f:	mov    ax,word ptr [bp-8]
     472:	sub    word ptr [bp-6],ax
     475:	sbb    word ptr [bp-4],dx
     478:	mov    cl,0x5
     47a:	mov    bx,word ptr [bp-40]
     47d:	mov    ax,word ptr [bx]
     47f:	shr    ax,cl
     481:	sub    word ptr [bx],ax
     483:	mov    ax,word ptr [bp-26]
     486:	add    ax,ax
     488:	inc    ax
     489:	mov    word ptr [bp-26],ax
     48c:	jmp    0x3a7
     48f:	mov    al,byte ptr [bp-26]
     492:	mov    byte ptr [bp-2],al
     495:	mov    bx,word ptr [bp-20]
     498:	mov    cx,word ptr [bp-18]
     49b:	add    word ptr [bp-20],0x1
     49f:	adc    word ptr [bp-18],0x0
     4a3:	mov    ax,word ptr [bp+20]
     4a6:	mov    dx,word ptr [bp+22]
     4a9:	call   0x4aa	4aa: R_386_PC16	__PIA
     4ac:	mov    bx,ax
     4ae:	movl   es,dx
     4b0:	mov    al,byte ptr [bp-26]
     4b3:	mov    byte ptr es:[bx],al
     4b6:	mov    ax,word ptr [bp-16]
     4b9:	cmp    ax,0x4
     4bc:	jge    0x4c6
     4be:	xor    ax,ax
     4c0:	mov    word ptr [bp-16],ax
     4c3:	jmp    0x153
     4c6:	cmp    ax,0xa
     4c9:	jge    0x4d2
     4cb:	sub    word ptr [bp-16],0x3
     4cf:	jmp    0x153
     4d2:	sub    word ptr [bp-16],0x6
     4d6:	jmp    0x153
     4d9:	sub    si,word ptr [bp-8]
     4dc:	sbb    di,dx
     4de:	mov    ax,word ptr [bp-8]
     4e1:	sub    word ptr [bp-6],ax
     4e4:	sbb    word ptr [bp-4],dx
     4e7:	mov    cl,0x5
     4e9:	mov    bx,word ptr [bp-14]
     4ec:	mov    ax,word ptr [bx]
     4ee:	shr    ax,cl
     4f0:	sub    word ptr [bx],ax
     4f2:	mov    ax,word ptr [bp-16]
     4f5:	shl    ax,1
     4f7:	mov    dx,word ptr [bp-34]
     4fa:	add    dx,0x180
     4fe:	add    dx,ax
     500:	mov    word ptr [bp-14],dx
     503:	cmp    di,0x100
     507:	jae    0x560
     509:	mov    ax,word ptr [bp-12]
     50c:	mov    dx,word ptr [bp-10]
     50f:	mov    bx,word ptr [bp-30]
     512:	mov    cx,word ptr [bp-28]
     515:	call   0x516	516: R_386_PC16	__PTC
     518:	jne    0x51d
     51a:	jmp    0x14d
     51d:	mov    cx,0x8
     520:	shl    si,1
     522:	rcl    di,1
     524:	loop   0x520
     526:	les    bx,dword ptr [bp-12]
     529:	mov    al,byte ptr es:[bx]
     52c:	mov    byte ptr [bp-90],al
     52f:	mov    byte ptr [bp-89],0x0
     533:	mov    ax,word ptr [bp-6]
     536:	mov    dx,word ptr [bp-4]
     539:	mov    cx,0x8
     53c:	shl    ax,1
     53e:	rcl    dx,1
     540:	loop   0x53c
     542:	mov    bx,word ptr [bp-90]
     545:	or     bx,ax
     547:	mov    word ptr [bp-6],bx
     54a:	mov    word ptr [bp-4],dx
     54d:	mov    ax,word ptr [bp-12]
     550:	movl   dx,es
     552:	mov    bx,0x1
     555:	xor    cx,cx
     557:	call   0x558	558: R_386_PC16	__PIA
     55a:	mov    word ptr [bp-12],ax
     55d:	mov    word ptr [bp-10],dx
     560:	mov    word ptr [bp-98],si
     563:	mov    word ptr [bp-96],di
     566:	mov    cx,0xb
     569:	shr    word ptr [bp-96],1
     56c:	rcr    word ptr [bp-98],1
     56f:	loop   0x569
     571:	mov    bx,word ptr [bp-14]
     574:	mov    bx,word ptr [bx]
     576:	mov    ax,word ptr [bp-98]
     579:	mov    dx,word ptr [bp-96]
     57c:	xor    cx,cx
     57e:	call   0x57f	57f: R_386_PC16	__U4M
     581:	mov    word ptr [bp-8],ax
     584:	mov    word ptr [bp-94],dx
     587:	mov    ax,word ptr [bp-4]
     58a:	cmp    ax,dx
     58c:	jb     0x598
     58e:	jne    0x5e9
     590:	mov    ax,word ptr [bp-6]
     593:	cmp    ax,word ptr [bp-8]
     596:	jae    0x5e9
     598:	mov    si,word ptr [bp-8]
     59b:	mov    di,dx
     59d:	mov    ax,0x800
     5a0:	mov    bx,word ptr [bp-14]
     5a3:	sub    ax,word ptr [bx]
     5a5:	mov    cl,0x5
     5a7:	shr    ax,cl
     5a9:	add    word ptr [bx],ax
     5ab:	mov    ax,word ptr [bp-56]
     5ae:	mov    word ptr [bp-68],ax
     5b1:	mov    ax,word ptr [bp-54]
     5b4:	mov    word ptr [bp-74],ax
     5b7:	mov    ax,word ptr [bp-52]
     5ba:	mov    word ptr [bp-56],ax
     5bd:	mov    ax,word ptr [bp-50]
     5c0:	mov    word ptr [bp-54],ax
     5c3:	mov    ax,word ptr [bp-102]
     5c6:	mov    word ptr [bp-52],ax
     5c9:	mov    ax,word ptr [bp-100]
     5cc:	mov    word ptr [bp-50],ax
     5cf:	cmp    word ptr [bp-16],0x7
     5d3:	jge    0x5d9
     5d5:	xor    ax,ax
     5d7:	jmp    0x5dc
     5d9:	mov    ax,0x3
     5dc:	mov    word ptr [bp-16],ax
     5df:	mov    bx,word ptr [bp-34]
     5e2:	add    bx,0x664
     5e6:	jmp    0xa1f
     5e9:	sub    si,word ptr [bp-8]
     5ec:	sbb    di,dx
     5ee:	mov    ax,word ptr [bp-8]
     5f1:	sub    word ptr [bp-6],ax
     5f4:	sbb    word ptr [bp-4],dx
     5f7:	mov    cl,0x5
     5f9:	mov    bx,word ptr [bp-14]
     5fc:	mov    ax,word ptr [bx]
     5fe:	shr    ax,cl
     600:	sub    word ptr [bx],ax
     602:	mov    dx,word ptr [bp-16]
     605:	shl    dx,1
     607:	mov    ax,word ptr [bp-34]
     60a:	add    ax,0x198
     60d:	add    ax,dx
     60f:	mov    word ptr [bp-14],ax
     612:	cmp    di,0x100
     616:	jae    0x66f
     618:	mov    ax,word ptr [bp-12]
     61b:	mov    dx,word ptr [bp-10]
     61e:	mov    bx,word ptr [bp-30]
     621:	mov    cx,word ptr [bp-28]
     624:	call   0x625	625: R_386_PC16	__PTC
     627:	jne    0x62c
     629:	jmp    0x14d
     62c:	mov    cx,0x8
     62f:	shl    si,1
     631:	rcl    di,1
     633:	loop   0x62f
     635:	les    bx,dword ptr [bp-12]
     638:	mov    al,byte ptr es:[bx]
     63b:	mov    byte ptr [bp-90],al
     63e:	mov    byte ptr [bp-89],0x0
     642:	mov    ax,word ptr [bp-6]
     645:	mov    dx,word ptr [bp-4]
     648:	mov    cx,0x8
     64b:	shl    ax,1
     64d:	rcl    dx,1
     64f:	loop   0x64b
     651:	mov    bx,word ptr [bp-90]
     654:	or     bx,ax
     656:	mov    word ptr [bp-6],bx
     659:	mov    word ptr [bp-4],dx
     65c:	mov    ax,word ptr [bp-12]
     65f:	movl   dx,es
     661:	mov    bx,0x1
     664:	xor    cx,cx
     666:	call   0x667	667: R_386_PC16	__PIA
     669:	mov    word ptr [bp-12],ax
     66c:	mov    word ptr [bp-10],dx
     66f:	mov    word ptr [bp-98],si
     672:	mov    word ptr [bp-96],di
     675:	mov    cx,0xb
     678:	shr    word ptr [bp-96],1
     67b:	rcr    word ptr [bp-98],1
     67e:	loop   0x678
     680:	mov    bx,word ptr [bp-14]
     683:	mov    bx,word ptr [bx]
     685:	mov    ax,word ptr [bp-98]
     688:	mov    dx,word ptr [bp-96]
     68b:	xor    cx,cx
     68d:	call   0x68e	68e: R_386_PC16	__U4M
     690:	mov    word ptr [bp-8],ax
     693:	mov    word ptr [bp-94],dx
     696:	mov    ax,word ptr [bp-12]
     699:	mov    dx,word ptr [bp-10]
     69c:	mov    bx,0x1
     69f:	xor    cx,cx
     6a1:	call   0x6a2	6a2: R_386_PC16	__PIA
     6a4:	mov    word ptr [bp-82],ax
     6a7:	mov    word ptr [bp-80],dx
     6aa:	mov    ax,word ptr [bp-4]
     6ad:	cmp    ax,word ptr [bp-94]
     6b0:	jb     0x6bf
     6b2:	je     0x6b7
     6b4:	jmp    0x80d
     6b7:	mov    ax,word ptr [bp-6]
     6ba:	cmp    ax,word ptr [bp-8]
     6bd:	jae    0x6b4
     6bf:	mov    si,word ptr [bp-8]
     6c2:	mov    di,word ptr [bp-94]
     6c5:	mov    ax,0x800
     6c8:	mov    bx,word ptr [bp-14]
     6cb:	sub    ax,word ptr [bx]
     6cd:	mov    cl,0x5
     6cf:	shr    ax,cl
     6d1:	add    word ptr [bx],ax
     6d3:	mov    ax,word ptr [bp-16]
     6d6:	shl    ax,cl
     6d8:	mov    dx,word ptr [bp-34]
     6db:	add    dx,0x1e0
     6df:	add    dx,ax
     6e1:	mov    ax,word ptr [bp-58]
     6e4:	shl    ax,1
     6e6:	add    dx,ax
     6e8:	mov    word ptr [bp-14],dx
     6eb:	cmp    di,0x100
     6ef:	jae    0x745
     6f1:	mov    ax,word ptr [bp-12]
     6f4:	mov    dx,word ptr [bp-10]
     6f7:	mov    bx,word ptr [bp-30]
     6fa:	mov    cx,word ptr [bp-28]
     6fd:	call   0x6fe	6fe: R_386_PC16	__PTC
     700:	jne    0x705
     702:	jmp    0x14d
     705:	mov    ax,si
     707:	mov    cx,0x8
     70a:	shl    ax,1
     70c:	rcl    di,1
     70e:	loop   0x70a
     710:	mov    si,ax
     712:	les    bx,dword ptr [bp-12]
     715:	mov    al,byte ptr es:[bx]
     718:	mov    byte ptr [bp-90],al
     71b:	mov    byte ptr [bp-89],0x0
     71f:	mov    ax,word ptr [bp-6]
     722:	mov    dx,word ptr [bp-4]
     725:	mov    cx,0x8
     728:	shl    ax,1
     72a:	rcl    dx,1
     72c:	loop   0x728
     72e:	mov    bx,word ptr [bp-90]
     731:	or     bx,ax
     733:	mov    word ptr [bp-6],bx
     736:	mov    word ptr [bp-4],dx
     739:	mov    bx,word ptr [bp-82]
     73c:	mov    word ptr [bp-12],bx
     73f:	mov    ax,word ptr [bp-80]
     742:	mov    word ptr [bp-10],ax
     745:	mov    word ptr [bp-98],si
     748:	mov    word ptr [bp-96],di
     74b:	mov    cx,0xb
     74e:	shr    word ptr [bp-96],1
     751:	rcr    word ptr [bp-98],1
     754:	loop   0x74e
     756:	mov    bx,word ptr [bp-14]
     759:	mov    bx,word ptr [bx]
     75b:	mov    ax,word ptr [bp-98]
     75e:	mov    dx,word ptr [bp-96]
     761:	xor    cx,cx
     763:	call   0x764	764: R_386_PC16	__U4M
     766:	mov    word ptr [bp-8],ax
     769:	mov    word ptr [bp-94],dx
     76c:	mov    ax,word ptr [bp-4]
     76f:	cmp    ax,dx
     771:	jb     0x77d
     773:	jne    0x7a6
     775:	mov    ax,word ptr [bp-6]
     778:	cmp    ax,word ptr [bp-8]
     77b:	jae    0x7f1
     77d:	mov    si,word ptr [bp-8]
     780:	mov    di,dx
     782:	mov    ax,0x800
     785:	mov    bx,word ptr [bp-14]
     788:	sub    ax,word ptr [bx]
     78a:	mov    cl,0x5
     78c:	shr    ax,cl
     78e:	add    word ptr [bx],ax
     790:	mov    ax,word ptr [bp-18]
     793:	or     ax,word ptr [bp-20]
     796:	jne    0x79b
     798:	jmp    0x14d
     79b:	cmp    word ptr [bp-16],0x7
     79f:	jge    0x7a8
     7a1:	mov    ax,0x9
     7a4:	jmp    0x7ab
     7a6:	jmp    0x7f1
     7a8:	mov    ax,0xb
     7ab:	mov    word ptr [bp-16],ax
     7ae:	mov    bx,word ptr [bp-20]
     7b1:	sub    bx,word ptr [bp-102]
     7b4:	mov    cx,word ptr [bp-18]
     7b7:	sbb    cx,word ptr [bp-100]
     7ba:	mov    ax,word ptr [bp+20]
     7bd:	mov    dx,word ptr [bp+22]
     7c0:	call   0x7c1	7c1: R_386_PC16	__PIA
     7c3:	mov    bx,ax
     7c5:	movl   es,dx
     7c7:	mov    al,byte ptr es:[bx]
     7ca:	mov    byte ptr [bp-2],al
     7cd:	mov    bx,word ptr [bp-20]
     7d0:	mov    cx,word ptr [bp-18]
     7d3:	add    word ptr [bp-20],0x1
     7d7:	adc    word ptr [bp-18],0x0
     7db:	mov    ax,word ptr [bp+20]
     7de:	mov    dx,word ptr [bp+22]
     7e1:	call   0x7e2	7e2: R_386_PC16	__PIA
     7e4:	mov    bx,ax
     7e6:	movl   es,dx
     7e8:	mov    al,byte ptr [bp-2]
     7eb:	mov    byte ptr es:[bx],al
     7ee:	jmp    0x153
     7f1:	sub    si,word ptr [bp-8]
     7f4:	sbb    di,dx
     7f6:	mov    ax,word ptr [bp-8]
     7f9:	sub    word ptr [bp-6],ax
     7fc:	sbb    word ptr [bp-4],dx
     7ff:	mov    cl,0x5
     801:	mov    bx,word ptr [bp-14]
     804:	mov    ax,word ptr [bx]
     806:	shr    ax,cl
     808:	sub    word ptr [bx],ax
     80a:	jmp    0xa07
     80d:	sub    si,word ptr [bp-8]
     810:	sbb    di,word ptr [bp-94]
     813:	mov    ax,word ptr [bp-8]
     816:	sub    word ptr [bp-6],ax
     819:	mov    ax,word ptr [bp-94]
     81c:	sbb    word ptr [bp-4],ax
     81f:	mov    cl,0x5
     821:	mov    bx,word ptr [bp-14]
     824:	mov    ax,word ptr [bx]
     826:	shr    ax,cl
     828:	sub    word ptr [bx],ax
     82a:	mov    ax,word ptr [bp-16]
     82d:	shl    ax,1
     82f:	mov    dx,word ptr [bp-34]
     832:	add    dx,0x1b0
     836:	add    dx,ax
     838:	mov    word ptr [bp-14],dx
     83b:	cmp    di,0x100
     83f:	jae    0x891
     841:	mov    ax,word ptr [bp-12]
     844:	mov    dx,word ptr [bp-10]
     847:	mov    bx,word ptr [bp-30]
     84a:	mov    cx,word ptr [bp-28]
     84d:	call   0x84e	84e: R_386_PC16	__PTC
     850:	jne    0x855
     852:	jmp    0x14d
     855:	mov    cx,0x8
     858:	shl    si,1
     85a:	rcl    di,1
     85c:	loop   0x858
     85e:	les    bx,dword ptr [bp-12]
     861:	mov    al,byte ptr es:[bx]
     864:	mov    byte ptr [bp-90],al
     867:	mov    byte ptr [bp-89],0x0
     86b:	mov    ax,word ptr [bp-6]
     86e:	mov    dx,word ptr [bp-4]
     871:	mov    cx,0x8
     874:	shl    ax,1
     876:	rcl    dx,1
     878:	loop   0x874
     87a:	mov    bx,word ptr [bp-90]
     87d:	or     bx,ax
     87f:	mov    word ptr [bp-6],bx
     882:	mov    word ptr [bp-4],dx
     885:	mov    bx,word ptr [bp-82]
     888:	mov    word ptr [bp-12],bx
     88b:	mov    ax,word ptr [bp-80]
     88e:	mov    word ptr [bp-10],ax
     891:	mov    word ptr [bp-98],si
     894:	mov    word ptr [bp-96],di
     897:	mov    cx,0xb
     89a:	shr    word ptr [bp-96],1
     89d:	rcr    word ptr [bp-98],1
     8a0:	loop   0x89a
     8a2:	mov    bx,word ptr [bp-14]
     8a5:	mov    bx,word ptr [bx]
     8a7:	mov    ax,word ptr [bp-98]
     8aa:	mov    dx,word ptr [bp-96]
     8ad:	xor    cx,cx
     8af:	call   0x8b0	8b0: R_386_PC16	__U4M
     8b2:	mov    word ptr [bp-8],ax
     8b5:	mov    word ptr [bp-94],dx
     8b8:	mov    ax,word ptr [bp-4]
     8bb:	cmp    ax,dx
     8bd:	jb     0x8c9
     8bf:	jne    0x8e5
     8c1:	mov    ax,word ptr [bp-6]
     8c4:	cmp    ax,word ptr [bp-8]
     8c7:	jae    0x8e5
     8c9:	mov    si,word ptr [bp-8]
     8cc:	mov    di,dx
     8ce:	mov    ax,0x800
     8d1:	mov    bx,word ptr [bp-14]
     8d4:	sub    ax,word ptr [bx]
     8d6:	mov    cl,0x5
     8d8:	shr    ax,cl
     8da:	add    word ptr [bx],ax
     8dc:	mov    ax,word ptr [bp-52]
     8df:	mov    cx,word ptr [bp-50]
     8e2:	jmp    0x9f5
     8e5:	sub    si,word ptr [bp-8]
     8e8:	sbb    di,dx
     8ea:	mov    ax,word ptr [bp-8]
     8ed:	sub    word ptr [bp-6],ax
     8f0:	sbb    word ptr [bp-4],dx
     8f3:	mov    cl,0x5
     8f5:	mov    bx,word ptr [bp-14]
     8f8:	mov    ax,word ptr [bx]
     8fa:	shr    ax,cl
     8fc:	sub    word ptr [bx],ax
     8fe:	mov    dx,word ptr [bp-16]
     901:	shl    dx,1
     903:	mov    ax,word ptr [bp-34]
     906:	add    ax,0x1c8
     909:	add    ax,dx
     90b:	mov    word ptr [bp-14],ax
     90e:	cmp    di,0x100
     912:	jae    0x96b
     914:	mov    ax,word ptr [bp-12]
     917:	mov    dx,word ptr [bp-10]
     91a:	mov    bx,word ptr [bp-30]
     91d:	mov    cx,word ptr [bp-28]
     920:	call   0x921	921: R_386_PC16	__PTC
     923:	jne    0x928
     925:	jmp    0x14d
     928:	mov    cx,0x8
     92b:	shl    si,1
     92d:	rcl    di,1
     92f:	loop   0x92b
     931:	les    bx,dword ptr [bp-12]
     934:	mov    al,byte ptr es:[bx]
     937:	mov    byte ptr [bp-90],al
     93a:	mov    byte ptr [bp-89],0x0
     93e:	mov    ax,word ptr [bp-6]
     941:	mov    dx,word ptr [bp-4]
     944:	mov    cx,0x8
     947:	shl    ax,1
     949:	rcl    dx,1
     94b:	loop   0x947
     94d:	mov    bx,word ptr [bp-90]
     950:	or     bx,ax
     952:	mov    word ptr [bp-6],bx
     955:	mov    word ptr [bp-4],dx
     958:	mov    ax,word ptr [bp-12]
     95b:	movl   dx,es
     95d:	mov    bx,0x1
     960:	xor    cx,cx
     962:	call   0x963	963: R_386_PC16	__PIA
     965:	mov    word ptr [bp-12],ax
     968:	mov    word ptr [bp-10],dx
     96b:	mov    word ptr [bp-98],si
     96e:	mov    word ptr [bp-96],di
     971:	mov    cx,0xb
     974:	shr    word ptr [bp-96],1
     977:	rcr    word ptr [bp-98],1
     97a:	loop   0x974
     97c:	mov    bx,word ptr [bp-14]
     97f:	mov    bx,word ptr [bx]
     981:	mov    ax,word ptr [bp-98]
     984:	mov    dx,word ptr [bp-96]
     987:	xor    cx,cx
     989:	call   0x98a	98a: R_386_PC16	__U4M
     98c:	mov    word ptr [bp-8],ax
     98f:	mov    word ptr [bp-94],dx
     992:	mov    ax,word ptr [bp-4]
     995:	cmp    ax,dx
     997:	jb     0x9a3
     999:	jne    0x9be
     99b:	mov    ax,word ptr [bp-6]
     99e:	cmp    ax,word ptr [bp-8]
     9a1:	jae    0x9be
     9a3:	mov    si,word ptr [bp-8]
     9a6:	mov    di,dx
     9a8:	mov    ax,0x800
     9ab:	mov    bx,word ptr [bp-14]
     9ae:	sub    ax,word ptr [bx]
     9b0:	mov    cl,0x5
     9b2:	shr    ax,cl
     9b4:	add    word ptr [bx],ax
     9b6:	mov    ax,word ptr [bp-56]
     9b9:	mov    cx,word ptr [bp-54]
     9bc:	jmp    0x9e9
     9be:	sub    si,word ptr [bp-8]
     9c1:	sbb    di,dx
     9c3:	mov    ax,word ptr [bp-8]
     9c6:	sub    word ptr [bp-6],ax
     9c9:	sbb    word ptr [bp-4],dx
     9cc:	mov    cl,0x5
     9ce:	mov    bx,word ptr [bp-14]
     9d1:	mov    ax,word ptr [bx]
     9d3:	shr    ax,cl
     9d5:	sub    word ptr [bx],ax
     9d7:	mov    ax,word ptr [bp-68]
     9da:	mov    cx,word ptr [bp-74]
     9dd:	mov    dx,word ptr [bp-56]
     9e0:	mov    word ptr [bp-68],dx
     9e3:	mov    dx,word ptr [bp-54]
     9e6:	mov    word ptr [bp-74],dx
     9e9:	mov    dx,word ptr [bp-52]
     9ec:	mov    word ptr [bp-56],dx
     9ef:	mov    dx,word ptr [bp-50]
     9f2:	mov    word ptr [bp-54],dx
     9f5:	mov    dx,word ptr [bp-102]
     9f8:	mov    word ptr [bp-52],dx
     9fb:	mov    dx,word ptr [bp-100]
     9fe:	mov    word ptr [bp-50],dx
     a01:	mov    word ptr [bp-102],ax
     a04:	mov    word ptr [bp-100],cx
     a07:	cmp    word ptr [bp-16],0x7
     a0b:	jge    0xa12
     a0d:	mov    ax,0x8
     a10:	jmp    0xa15
     a12:	mov    ax,0xb
     a15:	mov    word ptr [bp-16],ax
     a18:	mov    bx,word ptr [bp-34]
     a1b:	add    bx,0xa68
     a1f:	mov    word ptr [bp-14],bx
     a22:	mov    bx,word ptr [bp-14]
     a25:	mov    word ptr [bp-22],bx
     a28:	cmp    di,0x100
     a2c:	jae    0xa85
     a2e:	mov    ax,word ptr [bp-12]
     a31:	mov    dx,word ptr [bp-10]
     a34:	mov    bx,word ptr [bp-30]
     a37:	mov    cx,word ptr [bp-28]
     a3a:	call   0xa3b	a3b: R_386_PC16	__PTC
     a3d:	jne    0xa42
     a3f:	jmp    0x14d
     a42:	mov    cx,0x8
     a45:	shl    si,1
     a47:	rcl    di,1
     a49:	loop   0xa45
     a4b:	les    bx,dword ptr [bp-12]
     a4e:	mov    al,byte ptr es:[bx]
     a51:	mov    byte ptr [bp-90],al
     a54:	mov    byte ptr [bp-89],0x0
     a58:	mov    ax,word ptr [bp-6]
     a5b:	mov    dx,word ptr [bp-4]
     a5e:	mov    cx,0x8
     a61:	shl    ax,1
     a63:	rcl    dx,1
     a65:	loop   0xa61
     a67:	mov    bx,word ptr [bp-90]
     a6a:	or     bx,ax
     a6c:	mov    word ptr [bp-6],bx
     a6f:	mov    word ptr [bp-4],dx
     a72:	mov    ax,word ptr [bp-12]
     a75:	movl   dx,es
     a77:	mov    bx,0x1
     a7a:	xor    cx,cx
     a7c:	call   0xa7d	a7d: R_386_PC16	__PIA
     a7f:	mov    word ptr [bp-12],ax
     a82:	mov    word ptr [bp-10],dx
     a85:	mov    word ptr [bp-98],si
     a88:	mov    word ptr [bp-96],di
     a8b:	mov    cx,0xb
     a8e:	shr    word ptr [bp-96],1
     a91:	rcr    word ptr [bp-98],1
     a94:	loop   0xa8e
     a96:	mov    bx,word ptr [bp-22]
     a99:	mov    bx,word ptr [bx]
     a9b:	mov    ax,word ptr [bp-98]
     a9e:	mov    dx,word ptr [bp-96]
     aa1:	xor    cx,cx
     aa3:	call   0xaa4	aa4: R_386_PC16	__U4M
     aa6:	mov    word ptr [bp-8],ax
     aa9:	mov    word ptr [bp-94],dx
     aac:	mov    ax,word ptr [bp-4]
     aaf:	cmp    ax,dx
     ab1:	jb     0xabd
     ab3:	jne    0xaef
     ab5:	mov    ax,word ptr [bp-6]
     ab8:	cmp    ax,word ptr [bp-8]
     abb:	jae    0xaef
     abd:	mov    si,word ptr [bp-8]
     ac0:	mov    di,dx
     ac2:	mov    ax,0x800
     ac5:	mov    bx,word ptr [bp-22]
     ac8:	sub    ax,word ptr [bx]
     aca:	mov    cl,0x5
     acc:	shr    ax,cl
     ace:	add    word ptr [bx],ax
     ad0:	mov    cl,0x4
     ad2:	mov    ax,word ptr [bp-58]
     ad5:	shl    ax,cl
     ad7:	mov    dx,word ptr [bp-14]
     ada:	add    dx,0x4
     add:	add    dx,ax
     adf:	mov    word ptr [bp-22],dx
     ae2:	xor    ax,ax
     ae4:	mov    word ptr [bp-70],ax
     ae7:	mov    word ptr [bp-60],0x3
     aec:	jmp    0xc00
     aef:	sub    si,word ptr [bp-8]
     af2:	sbb    di,dx
     af4:	mov    ax,word ptr [bp-8]
     af7:	sub    word ptr [bp-6],ax
     afa:	sbb    word ptr [bp-4],dx
     afd:	mov    cl,0x5
     aff:	mov    bx,word ptr [bp-22]
     b02:	mov    ax,word ptr [bx]
     b04:	shr    ax,cl
     b06:	sub    word ptr [bx],ax
     b08:	mov    bx,word ptr [bp-14]
     b0b:	inc    bx
     b0c:	inc    bx
     b0d:	mov    word ptr [bp-22],bx
     b10:	cmp    di,0x100
     b14:	jae    0xb6d
     b16:	mov    ax,word ptr [bp-12]
     b19:	mov    dx,word ptr [bp-10]
     b1c:	mov    bx,word ptr [bp-30]
     b1f:	mov    cx,word ptr [bp-28]
     b22:	call   0xb23	b23: R_386_PC16	__PTC
     b25:	jne    0xb2a
     b27:	jmp    0x14d
     b2a:	mov    cx,0x8
     b2d:	shl    si,1
     b2f:	rcl    di,1
     b31:	loop   0xb2d
     b33:	les    bx,dword ptr [bp-12]
     b36:	mov    al,byte ptr es:[bx]
     b39:	mov    byte ptr [bp-90],al
     b3c:	mov    byte ptr [bp-89],0x0
     b40:	mov    ax,word ptr [bp-6]
     b43:	mov    dx,word ptr [bp-4]
     b46:	mov    cx,0x8
     b49:	shl    ax,1
     b4b:	rcl    dx,1
     b4d:	loop   0xb49
     b4f:	mov    bx,word ptr [bp-90]
     b52:	or     bx,ax
     b54:	mov    word ptr [bp-6],bx
     b57:	mov    word ptr [bp-4],dx
     b5a:	mov    ax,word ptr [bp-12]
     b5d:	movl   dx,es
     b5f:	mov    bx,0x1
     b62:	xor    cx,cx
     b64:	call   0xb65	b65: R_386_PC16	__PIA
     b67:	mov    word ptr [bp-12],ax
     b6a:	mov    word ptr [bp-10],dx
     b6d:	mov    word ptr [bp-98],si
     b70:	mov    word ptr [bp-96],di
     b73:	mov    cx,0xb
     b76:	shr    word ptr [bp-96],1
     b79:	rcr    word ptr [bp-98],1
     b7c:	loop   0xb76
     b7e:	mov    bx,word ptr [bp-22]
     b81:	mov    bx,word ptr [bx]
     b83:	mov    ax,word ptr [bp-98]
     b86:	mov    dx,word ptr [bp-96]
     b89:	xor    cx,cx
     b8b:	call   0xb8c	b8c: R_386_PC16	__U4M
     b8e:	mov    word ptr [bp-8],ax
     b91:	mov    word ptr [bp-94],dx
     b94:	mov    ax,word ptr [bp-4]
     b97:	cmp    ax,dx
     b99:	jb     0xba5
     b9b:	jne    0xbd3
     b9d:	mov    ax,word ptr [bp-6]
     ba0:	cmp    ax,word ptr [bp-8]
     ba3:	jae    0xbd3
     ba5:	mov    si,word ptr [bp-8]
     ba8:	mov    di,dx
     baa:	mov    ax,0x800
     bad:	mov    bx,word ptr [bp-22]
     bb0:	sub    ax,word ptr [bx]
     bb2:	mov    cl,0x5
     bb4:	shr    ax,cl
     bb6:	add    word ptr [bx],ax
     bb8:	mov    cl,0x4
     bba:	mov    ax,word ptr [bp-58]
     bbd:	shl    ax,cl
     bbf:	mov    dx,word ptr [bp-14]
     bc2:	add    dx,0x104
     bc6:	add    dx,ax
     bc8:	mov    word ptr [bp-22],dx
     bcb:	mov    word ptr [bp-70],0x8
     bd0:	jmp    0xae7
     bd3:	sub    si,word ptr [bp-8]
     bd6:	sbb    di,dx
     bd8:	mov    ax,word ptr [bp-8]
     bdb:	sub    word ptr [bp-6],ax
     bde:	sbb    word ptr [bp-4],dx
     be1:	mov    cl,0x5
     be3:	mov    bx,word ptr [bp-22]
     be6:	mov    ax,word ptr [bx]
     be8:	shr    ax,cl
     bea:	sub    word ptr [bx],ax
     bec:	mov    bx,word ptr [bp-14]
     bef:	add    bx,0x204
     bf3:	mov    word ptr [bp-22],bx
     bf6:	mov    word ptr [bp-70],0x10
     bfb:	mov    word ptr [bp-60],0x8
     c00:	mov    ax,word ptr [bp-60]
     c03:	mov    word ptr [bp-62],ax
     c06:	mov    word ptr [bp-24],0x1
     c0b:	mov    ax,word ptr [bp-24]
     c0e:	shl    ax,1
     c10:	mov    bx,word ptr [bp-22]
     c13:	add    bx,ax
     c15:	mov    word ptr [bp-38],bx
     c18:	cmp    di,0x100
     c1c:	jae    0xc75
     c1e:	mov    ax,word ptr [bp-12]
     c21:	mov    dx,word ptr [bp-10]
     c24:	mov    bx,word ptr [bp-30]
     c27:	mov    cx,word ptr [bp-28]
     c2a:	call   0xc2b	c2b: R_386_PC16	__PTC
     c2d:	jne    0xc32
     c2f:	jmp    0x14d
     c32:	mov    cx,0x8
     c35:	shl    si,1
     c37:	rcl    di,1
     c39:	loop   0xc35
     c3b:	les    bx,dword ptr [bp-12]
     c3e:	mov    al,byte ptr es:[bx]
     c41:	mov    byte ptr [bp-90],al
     c44:	mov    byte ptr [bp-89],0x0
     c48:	mov    ax,word ptr [bp-6]
     c4b:	mov    dx,word ptr [bp-4]
     c4e:	mov    cx,0x8
     c51:	shl    ax,1
     c53:	rcl    dx,1
     c55:	loop   0xc51
     c57:	mov    bx,word ptr [bp-90]
     c5a:	or     bx,ax
     c5c:	mov    word ptr [bp-6],bx
     c5f:	mov    word ptr [bp-4],dx
     c62:	mov    ax,word ptr [bp-12]
     c65:	movl   dx,es
     c67:	mov    bx,0x1
     c6a:	xor    cx,cx
     c6c:	call   0xc6d	c6d: R_386_PC16	__PIA
     c6f:	mov    word ptr [bp-12],ax
     c72:	mov    word ptr [bp-10],dx
     c75:	mov    word ptr [bp-98],si
     c78:	mov    word ptr [bp-96],di
     c7b:	mov    cx,0xb
     c7e:	shr    word ptr [bp-96],1
     c81:	rcr    word ptr [bp-98],1
     c84:	loop   0xc7e
     c86:	mov    bx,word ptr [bp-38]
     c89:	mov    bx,word ptr [bx]
     c8b:	mov    ax,word ptr [bp-98]
     c8e:	mov    dx,word ptr [bp-96]
     c91:	xor    cx,cx
     c93:	call   0xc94	c94: R_386_PC16	__U4M
     c96:	mov    word ptr [bp-8],ax
     c99:	mov    word ptr [bp-94],dx
     c9c:	mov    ax,word ptr [bp-4]
     c9f:	cmp    ax,dx
     ca1:	jb     0xcad
     ca3:	jne    0xcc5
     ca5:	mov    ax,word ptr [bp-6]
     ca8:	cmp    ax,word ptr [bp-8]
     cab:	jae    0xcc5
     cad:	mov    si,word ptr [bp-8]
     cb0:	mov    di,dx
     cb2:	mov    ax,0x800
     cb5:	mov    bx,word ptr [bp-38]
     cb8:	sub    ax,word ptr [bx]
     cba:	mov    cl,0x5
     cbc:	shr    ax,cl
     cbe:	add    word ptr [bx],ax
     cc0:	shl    word ptr [bp-24],1
     cc3:	jmp    0xce7
     cc5:	sub    si,word ptr [bp-8]
     cc8:	sbb    di,dx
     cca:	mov    ax,word ptr [bp-8]
     ccd:	sub    word ptr [bp-6],ax
     cd0:	sbb    word ptr [bp-4],dx
     cd3:	mov    cl,0x5
     cd5:	mov    bx,word ptr [bp-38]
     cd8:	mov    ax,word ptr [bx]
     cda:	shr    ax,cl
     cdc:	sub    word ptr [bx],ax
     cde:	mov    ax,word ptr [bp-24]
     ce1:	add    ax,ax
     ce3:	inc    ax
     ce4:	mov    word ptr [bp-24],ax
     ce7:	dec    word ptr [bp-62]
     cea:	je     0xcef
     cec:	jmp    0xc0b
     cef:	mov    cl,byte ptr [bp-60]
     cf2:	mov    ax,0x1
     cf5:	shl    ax,cl
     cf7:	sub    word ptr [bp-24],ax
     cfa:	mov    ax,word ptr [bp-70]
     cfd:	add    word ptr [bp-24],ax
     d00:	cmp    word ptr [bp-16],0x4
     d04:	jl     0xd09
     d06:	jmp    0x1025
     d09:	add    word ptr [bp-16],0x7
     d0d:	mov    ax,word ptr [bp-24]
     d10:	cmp    ax,0x4
     d13:	jl     0xd18
     d15:	mov    ax,0x3
     d18:	mov    cl,0x7
     d1a:	mov    dx,ax
     d1c:	shl    dx,cl
     d1e:	mov    ax,word ptr [bp-34]
     d21:	add    ax,0x360
     d24:	add    ax,dx
     d26:	mov    word ptr [bp-14],ax
     d29:	mov    word ptr [bp-92],0x6
     d2e:	mov    word ptr [bp-32],0x1
     d33:	mov    ax,word ptr [bp-32]
     d36:	shl    ax,1
     d38:	mov    bx,word ptr [bp-14]
     d3b:	add    bx,ax
     d3d:	mov    word ptr [bp-42],bx
     d40:	cmp    di,0x100
     d44:	jae    0xd9d
     d46:	mov    ax,word ptr [bp-12]
     d49:	mov    dx,word ptr [bp-10]
     d4c:	mov    bx,word ptr [bp-30]
     d4f:	mov    cx,word ptr [bp-28]
     d52:	call   0xd53	d53: R_386_PC16	__PTC
     d55:	jne    0xd5a
     d57:	jmp    0x14d
     d5a:	mov    cx,0x8
     d5d:	shl    si,1
     d5f:	rcl    di,1
     d61:	loop   0xd5d
     d63:	les    bx,dword ptr [bp-12]
     d66:	mov    al,byte ptr es:[bx]
     d69:	mov    byte ptr [bp-90],al
     d6c:	mov    byte ptr [bp-89],0x0
     d70:	mov    ax,word ptr [bp-6]
     d73:	mov    dx,word ptr [bp-4]
     d76:	mov    cx,0x8
     d79:	shl    ax,1
     d7b:	rcl    dx,1
     d7d:	loop   0xd79
     d7f:	mov    bx,word ptr [bp-90]
     d82:	or     bx,ax
     d84:	mov    word ptr [bp-6],bx
     d87:	mov    word ptr [bp-4],dx
     d8a:	mov    ax,word ptr [bp-12]
     d8d:	movl   dx,es
     d8f:	mov    bx,0x1
     d92:	xor    cx,cx
     d94:	call   0xd95	d95: R_386_PC16	__PIA
     d97:	mov    word ptr [bp-12],ax
     d9a:	mov    word ptr [bp-10],dx
     d9d:	mov    word ptr [bp-98],si
     da0:	mov    word ptr [bp-96],di
     da3:	mov    cx,0xb
     da6:	shr    word ptr [bp-96],1
     da9:	rcr    word ptr [bp-98],1
     dac:	loop   0xda6
     dae:	mov    bx,word ptr [bp-42]
     db1:	mov    bx,word ptr [bx]
     db3:	mov    ax,word ptr [bp-98]
     db6:	mov    dx,word ptr [bp-96]
     db9:	xor    cx,cx
     dbb:	call   0xdbc	dbc: R_386_PC16	__U4M
     dbe:	mov    word ptr [bp-8],ax
     dc1:	mov    word ptr [bp-94],dx
     dc4:	mov    ax,word ptr [bp-4]
     dc7:	cmp    ax,dx
     dc9:	jb     0xdd5
     dcb:	jne    0xded
     dcd:	mov    ax,word ptr [bp-6]
     dd0:	cmp    ax,word ptr [bp-8]
     dd3:	jae    0xded
     dd5:	mov    si,word ptr [bp-8]
     dd8:	mov    di,dx
     dda:	mov    ax,0x800
     ddd:	mov    bx,word ptr [bp-42]
     de0:	sub    ax,word ptr [bx]
     de2:	mov    cl,0x5
     de4:	shr    ax,cl
     de6:	add    word ptr [bx],ax
     de8:	shl    word ptr [bp-32],1
     deb:	jmp    0xe0f
     ded:	sub    si,word ptr [bp-8]
     df0:	sbb    di,dx
     df2:	mov    ax,word ptr [bp-8]
     df5:	sub    word ptr [bp-6],ax
     df8:	sbb    word ptr [bp-4],dx
     dfb:	mov    cl,0x5
     dfd:	mov    bx,word ptr [bp-42]
     e00:	mov    ax,word ptr [bx]
     e02:	shr    ax,cl
     e04:	sub    word ptr [bx],ax
     e06:	mov    ax,word ptr [bp-32]
     e09:	add    ax,ax
     e0b:	inc    ax
     e0c:	mov    word ptr [bp-32],ax
     e0f:	dec    word ptr [bp-92]
     e12:	je     0xe17
     e14:	jmp    0xd33
     e17:	sub    word ptr [bp-32],0x40
     e1b:	mov    ax,word ptr [bp-32]
     e1e:	cmp    ax,0x4
     e21:	jl     0xe68
     e23:	sar    ax,1
     e25:	dec    ax
     e26:	mov    word ptr [bp-36],ax
     e29:	mov    ax,word ptr [bp-32]
     e2c:	and    ax,0x1
     e2f:	or     al,0x2
     e31:	mov    word ptr [bp-102],ax
     e34:	xor    ax,ax
     e36:	mov    word ptr [bp-100],ax
     e39:	cmp    word ptr [bp-32],0xe
     e3d:	jge    0xe6b
     e3f:	mov    cx,word ptr [bp-36]
     e42:	jcxz   0xe4c
     e44:	shl    word ptr [bp-102],1
     e47:	rcl    word ptr [bp-100],1
     e4a:	loop   0xe44
     e4c:	mov    dx,word ptr [bp-102]
     e4f:	shl    dx,1
     e51:	mov    ax,word ptr [bp-34]
     e54:	add    ax,0x560
     e57:	add    dx,ax
     e59:	mov    ax,word ptr [bp-32]
     e5c:	shl    ax,1
     e5e:	sub    dx,ax
     e60:	dec    dx
     e61:	dec    dx
     e62:	mov    word ptr [bp-14],dx
     e65:	jmp    0xf10
     e68:	jmp    0x100c
     e6b:	sub    word ptr [bp-36],0x4
     e6f:	cmp    di,0x100
     e73:	jae    0xecc
     e75:	mov    ax,word ptr [bp-12]
     e78:	mov    dx,word ptr [bp-10]
     e7b:	mov    bx,word ptr [bp-30]
     e7e:	mov    cx,word ptr [bp-28]
     e81:	call   0xe82	e82: R_386_PC16	__PTC
     e84:	jne    0xe89
     e86:	jmp    0x14d
     e89:	mov    cx,0x8
     e8c:	shl    si,1
     e8e:	rcl    di,1
     e90:	loop   0xe8c
     e92:	les    bx,dword ptr [bp-12]
     e95:	mov    al,byte ptr es:[bx]
     e98:	mov    byte ptr [bp-90],al
     e9b:	mov    byte ptr [bp-89],0x0
     e9f:	mov    ax,word ptr [bp-6]
     ea2:	mov    dx,word ptr [bp-4]
     ea5:	mov    cx,0x8
     ea8:	shl    ax,1
     eaa:	rcl    dx,1
     eac:	loop   0xea8
     eae:	mov    bx,word ptr [bp-90]
     eb1:	or     bx,ax
     eb3:	mov    word ptr [bp-6],bx
     eb6:	mov    word ptr [bp-4],dx
     eb9:	mov    ax,word ptr [bp-12]
     ebc:	movl   dx,es
     ebe:	mov    bx,0x1
     ec1:	xor    cx,cx
     ec3:	call   0xec4	ec4: R_386_PC16	__PIA
     ec6:	mov    word ptr [bp-12],ax
     ec9:	mov    word ptr [bp-10],dx
     ecc:	shr    di,1
     ece:	rcr    si,1
     ed0:	shl    word ptr [bp-102],1
     ed3:	rcl    word ptr [bp-100],1
     ed6:	mov    ax,word ptr [bp-4]
     ed9:	cmp    di,ax
     edb:	jb     0xee4
     edd:	jne    0xeee
     edf:	cmp    si,word ptr [bp-6]
     ee2:	ja     0xeee
     ee4:	sub    word ptr [bp-6],si
     ee7:	sbb    word ptr [bp-4],di
     eea:	or     byte ptr [bp-102],0x1
     eee:	dec    word ptr [bp-36]
     ef1:	je     0xef6
     ef3:	jmp    0xe6f
     ef6:	mov    bx,word ptr [bp-34]
     ef9:	add    bx,0x644
     efd:	mov    word ptr [bp-14],bx
     f00:	mov    cx,0x4
     f03:	shl    word ptr [bp-102],1
     f06:	rcl    word ptr [bp-100],1
     f09:	loop   0xf03
     f0b:	mov    word ptr [bp-36],0x4
     f10:	mov    ax,0x1
     f13:	mov    word ptr [bp-64],ax
     f16:	mov    word ptr [bp-48],ax
     f19:	mov    ax,word ptr [bp-48]
     f1c:	shl    ax,1
     f1e:	mov    bx,word ptr [bp-14]
     f21:	add    bx,ax
     f23:	mov    word ptr [bp-44],bx
     f26:	cmp    di,0x100
     f2a:	jae    0xf83
     f2c:	mov    ax,word ptr [bp-12]
     f2f:	mov    dx,word ptr [bp-10]
     f32:	mov    bx,word ptr [bp-30]
     f35:	mov    cx,word ptr [bp-28]
     f38:	call   0xf39	f39: R_386_PC16	__PTC
     f3b:	jne    0xf40
     f3d:	jmp    0x14d
     f40:	mov    cx,0x8
     f43:	shl    si,1
     f45:	rcl    di,1
     f47:	loop   0xf43
     f49:	les    bx,dword ptr [bp-12]
     f4c:	mov    al,byte ptr es:[bx]
     f4f:	mov    byte ptr [bp-90],al
     f52:	mov    byte ptr [bp-89],0x0
     f56:	mov    ax,word ptr [bp-6]
     f59:	mov    dx,word ptr [bp-4]
     f5c:	mov    cx,0x8
     f5f:	shl    ax,1
     f61:	rcl    dx,1
     f63:	loop   0xf5f
     f65:	mov    bx,word ptr [bp-90]
     f68:	or     bx,ax
     f6a:	mov    word ptr [bp-6],bx
     f6d:	mov    word ptr [bp-4],dx
     f70:	mov    ax,word ptr [bp-12]
     f73:	movl   dx,es
     f75:	mov    bx,0x1
     f78:	xor    cx,cx
     f7a:	call   0xf7b	f7b: R_386_PC16	__PIA
     f7d:	mov    word ptr [bp-12],ax
     f80:	mov    word ptr [bp-10],dx
     f83:	mov    word ptr [bp-98],si
     f86:	mov    word ptr [bp-96],di
     f89:	mov    cx,0xb
     f8c:	shr    word ptr [bp-96],1
     f8f:	rcr    word ptr [bp-98],1
     f92:	loop   0xf8c
     f94:	mov    bx,word ptr [bp-44]
     f97:	mov    bx,word ptr [bx]
     f99:	mov    ax,word ptr [bp-98]
     f9c:	mov    dx,word ptr [bp-96]
     f9f:	xor    cx,cx
     fa1:	call   0xfa2	fa2: R_386_PC16	__U4M
     fa4:	mov    word ptr [bp-8],ax
     fa7:	mov    word ptr [bp-94],dx
     faa:	mov    ax,word ptr [bp-4]
     fad:	cmp    ax,dx
     faf:	jb     0xfbb
     fb1:	jne    0xfd3
     fb3:	mov    ax,word ptr [bp-6]
     fb6:	cmp    ax,word ptr [bp-8]
     fb9:	jae    0xfd3
     fbb:	mov    si,word ptr [bp-8]
     fbe:	mov    di,dx
     fc0:	mov    ax,0x800
     fc3:	mov    bx,word ptr [bp-44]
     fc6:	sub    ax,word ptr [bx]
     fc8:	mov    cl,0x5
     fca:	shr    ax,cl
     fcc:	add    word ptr [bx],ax
     fce:	shl    word ptr [bp-48],1
     fd1:	jmp    0xfff
     fd3:	sub    si,word ptr [bp-8]
     fd6:	sbb    di,dx
     fd8:	mov    ax,word ptr [bp-8]
     fdb:	sub    word ptr [bp-6],ax
     fde:	sbb    word ptr [bp-4],dx
     fe1:	mov    cl,0x5
     fe3:	mov    bx,word ptr [bp-44]
     fe6:	mov    ax,word ptr [bx]
     fe8:	shr    ax,cl
     fea:	sub    word ptr [bx],ax
     fec:	mov    ax,word ptr [bp-48]
     fef:	add    ax,ax
     ff1:	inc    ax
     ff2:	mov    word ptr [bp-48],ax
     ff5:	mov    ax,word ptr [bp-64]
     ff8:	cwd
     ff9:	or     word ptr [bp-102],ax
     ffc:	or     word ptr [bp-100],dx
     fff:	shl    word ptr [bp-64],1
    1002:	dec    word ptr [bp-36]
    1005:	je     0x100a
    1007:	jmp    0xf19
    100a:	jmp    0x1013
    100c:	cwd
    100d:	mov    word ptr [bp-102],ax
    1010:	mov    word ptr [bp-100],dx
    1013:	add    word ptr [bp-102],0x1
    1017:	adc    word ptr [bp-100],0x0
    101b:	mov    ax,word ptr [bp-100]
    101e:	or     ax,word ptr [bp-102]
    1021:	jne    0x1025
    1023:	jmp    0x109e
    1025:	add    word ptr [bp-24],0x2
    1029:	mov    ax,word ptr [bp-100]
    102c:	cmp    ax,word ptr [bp-18]
    102f:	jbe    0x1034
    1031:	jmp    0x14d
    1034:	jne    0x103e
    1036:	mov    ax,word ptr [bp-102]
    1039:	cmp    ax,word ptr [bp-20]
    103c:	ja     0x1031
    103e:	mov    bx,word ptr [bp-20]
    1041:	sub    bx,word ptr [bp-102]
    1044:	mov    cx,word ptr [bp-18]
    1047:	sbb    cx,word ptr [bp-100]
    104a:	mov    ax,word ptr [bp+20]
    104d:	mov    dx,word ptr [bp+22]
    1050:	call   0x1051	1051: R_386_PC16	__PIA
    1053:	mov    bx,ax
    1055:	movl   es,dx
    1057:	mov    al,byte ptr es:[bx]
    105a:	mov    byte ptr [bp-2],al
    105d:	mov    bx,word ptr [bp-20]
    1060:	mov    cx,word ptr [bp-18]
    1063:	dec    word ptr [bp-24]
    1066:	add    word ptr [bp-20],0x1
    106a:	adc    word ptr [bp-18],0x0
    106e:	mov    ax,word ptr [bp+20]
    1071:	mov    dx,word ptr [bp+22]
    1074:	call   0x1075	1075: R_386_PC16	__PIA
    1077:	mov    bx,ax
    1079:	movl   es,dx
    107b:	mov    al,byte ptr [bp-2]
    107e:	mov    byte ptr es:[bx],al
    1081:	cmp    word ptr [bp-24],0x0
    1085:	jne    0x108a
    1087:	jmp    0x153
    108a:	mov    ax,word ptr [bp-18]
    108d:	cmp    ax,word ptr [bp+26]
    1090:	jb     0x103e
    1092:	jne    0x1087
    1094:	mov    ax,word ptr [bp-20]
    1097:	cmp    ax,word ptr [bp+24]
    109a:	jb     0x103e
    109c:	jmp    0x1087
    109e:	cmp    di,0x100
    10a2:	jae    0x10cc
    10a4:	mov    ax,word ptr [bp-12]
    10a7:	mov    dx,word ptr [bp-10]
    10aa:	mov    bx,word ptr [bp-30]
    10ad:	mov    cx,word ptr [bp-28]
    10b0:	call   0x10b1	10b1: R_386_PC16	__PTC
    10b3:	jne    0x10b8
    10b5:	jmp    0x14d
    10b8:	mov    ax,word ptr [bp-12]
    10bb:	mov    dx,word ptr [bp-10]
    10be:	mov    bx,0x1
    10c1:	xor    cx,cx
    10c3:	call   0x10c4	10c4: R_386_PC16	__PIA
    10c6:	mov    word ptr [bp-12],ax
    10c9:	mov    word ptr [bp-10],dx
    10cc:	mov    ax,word ptr [bp-12]
    10cf:	mov    dx,word ptr [bp-10]
    10d2:	mov    bx,word ptr [bp+10]
    10d5:	mov    cx,word ptr [bp+12]
    10d8:	call   0x10d9	10d9: R_386_PC16	__PTS
    10db:	mov    bx,word ptr [bp+18]
    10de:	mov    word ptr [bx],ax
    10e0:	mov    word ptr [bx+2],dx
    10e3:	mov    ax,word ptr [bp-20]
    10e6:	mov    bx,word ptr [bp+28]
    10e9:	mov    word ptr [bx],ax
    10eb:	mov    ax,word ptr [bp-18]
    10ee:	mov    word ptr [bx+2],ax
    10f1:	xor    ax,ax
    10f3:	mov    sp,bp
    10f5:	pop    bp
    10f6:	pop    di
    10f7:	pop    si
    10f8:	ret
