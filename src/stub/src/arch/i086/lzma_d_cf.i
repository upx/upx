
tmp/lzma_d_cf.o:     file format coff-i386

Disassembly of section .text:

0000000000000000 <_LzmaDecode>:
       0:	push   si
       1:	push   di
       2:	push   bp
       3:	mov    bp,sp
       5:	sub    sp,0x6a
       8:	lds    si,DWORD PTR [bp+8]
       b:	movw    WORD PTR [bp-34],ds
       e:	lea    ax,[si+4]
      11:	mov    WORD PTR [bp-36],ax
      14:	xor    ax,ax
      16:	mov    WORD PTR [bp-20],ax
      19:	mov    WORD PTR [bp-96],ax
      1c:	mov    BYTE PTR [bp-2],0x0
      20:	mov    cl,BYTE PTR [si+2]
      23:	mov    ax,0x1
      26:	shl    ax,cl
      28:	dec    ax
      29:	mov    WORD PTR [bp-88],ax
      2c:	mov    cl,BYTE PTR [si+1]
      2f:	mov    ax,0x1
      32:	shl    ax,cl
      34:	dec    ax
      35:	cwd
      36:	mov    WORD PTR [bp-92],ax
      39:	mov    WORD PTR [bp-90],dx
      3c:	mov    al,BYTE PTR [si]
      3e:	xor    ah,ah
      40:	mov    WORD PTR [bp-82],ax
      43:	mov    WORD PTR [bp-18],0x0
      48:	mov    WORD PTR [bp-106],0x1
      4d:	mov    WORD PTR [bp-104],0x0
      52:	mov    WORD PTR [bp-58],0x1
      57:	mov    WORD PTR [bp-52],0x0
      5c:	mov    WORD PTR [bp-56],0x1
      61:	mov    WORD PTR [bp-54],0x0
      66:	mov    WORD PTR [bp-76],0x1
      6b:	mov    WORD PTR [bp-64],0x0
      70:	les    bx,DWORD PTR [bp+20]
      73:	mov    WORD PTR es:[bx],0x0
      78:	mov    WORD PTR es:[bx+2],0x0
      7e:	les    bx,DWORD PTR [bp+32]
      81:	mov    WORD PTR es:[bx],0x0
      86:	mov    WORD PTR es:[bx+2],0x0
      8c:	mov    dl,BYTE PTR [si+1]
      8f:	xor    dh,dh
      91:	mov    cx,ax
      93:	add    cx,dx
      95:	mov    ax,0x300
      98:	xor    dl,dl
      9a:	jcxz   a2 <_LzmaDecode+0xa2>
      9c:	shl    ax,1
      9e:	rcl    dx,1
      a0:	loop   9c <_LzmaDecode+0x9c>
      a2:	mov    si,ax
      a4:	add    si,0x736
      a8:	mov    cx,dx
      aa:	adc    cx,0x0
      ad:	xor    dx,dx
      af:	xor    ax,ax
      b1:	movw    ds,WORD PTR [bp-34]
      b4:	cmp    ax,cx
      b6:	jb     be <_LzmaDecode+0xbe>
      b8:	jne    d1 <_LzmaDecode+0xd1>
      ba:	cmp    dx,si
      bc:	jae    d1 <_LzmaDecode+0xd1>
      be:	mov    bx,dx
      c0:	shl    bx,1
      c2:	add    bx,WORD PTR [bp-36]
      c5:	mov    WORD PTR [bx],0x400
      c9:	add    dx,0x1
      cc:	adc    ax,0x0
      cf:	jmp    b4 <_LzmaDecode+0xb4>
      d1:	mov    bx,WORD PTR [bp+12]
      d4:	mov    WORD PTR [bp-12],bx
      d7:	movw    ds,WORD PTR [bp+14]
      da:	mov    ax,bx
      dc:	movl   dx,ds
      de:	mov    bx,WORD PTR [bp+16]
      e1:	mov    cx,WORD PTR [bp+18]
      e4:	call   e7 <_LzmaDecode+0xe7>	e5: 2	__PIA
      e7:	mov    WORD PTR [bp-26],ax
      ea:	mov    WORD PTR [bp-24],dx
      ed:	xor    ax,ax
      ef:	mov    WORD PTR [bp-8],ax
      f2:	mov    WORD PTR [bp-4],ax
      f5:	mov    si,0xffff
      f8:	mov    di,si
      fa:	mov    WORD PTR [bp-66],ax
      fd:	jmp    140 <_LzmaDecode+0x140>
      ff:	mov    bx,WORD PTR [bp-12]
     102:	mov    al,BYTE PTR [bx]
     104:	mov    BYTE PTR [bp-94],al
     107:	mov    BYTE PTR [bp-93],0x0
     10b:	mov    ax,WORD PTR [bp-8]
     10e:	mov    dx,WORD PTR [bp-4]
     111:	mov    cx,0x8
     114:	shl    ax,1
     116:	rcl    dx,1
     118:	loop   114 <_LzmaDecode+0x114>
     11a:	mov    bx,WORD PTR [bp-94]
     11d:	or     bx,ax
     11f:	mov    WORD PTR [bp-8],bx
     122:	mov    WORD PTR [bp-4],dx
     125:	mov    ax,WORD PTR [bp-12]
     128:	movl   dx,ds
     12a:	mov    bx,0x1
     12d:	xor    cx,cx
     12f:	call   132 <_LzmaDecode+0x132>	130: 2	__PIA
     132:	mov    WORD PTR [bp-12],ax
     135:	movl   ds,dx
     137:	inc    WORD PTR [bp-66]
     13a:	cmp    WORD PTR [bp-66],0x5
     13e:	jge    156 <_LzmaDecode+0x156>
     140:	mov    ax,WORD PTR [bp-12]
     143:	movl   dx,ds
     145:	mov    bx,WORD PTR [bp-26]
     148:	mov    cx,WORD PTR [bp-24]
     14b:	call   14e <_LzmaDecode+0x14e>	14c: 2	__PTC
     14e:	jne    ff <_LzmaDecode+0xff>
     150:	mov    ax,0x1
     153:	jmp    1183 <_LzmaDecode+0x1183>
     156:	mov    ax,WORD PTR [bp-96]
     159:	cmp    ax,WORD PTR [bp+30]
     15c:	jb     16b <_LzmaDecode+0x16b>
     15e:	je     163 <_LzmaDecode+0x163>
     160:	jmp    1132 <_LzmaDecode+0x1132>
     163:	mov    ax,WORD PTR [bp-20]
     166:	cmp    ax,WORD PTR [bp+28]
     169:	jae    160 <_LzmaDecode+0x160>
     16b:	mov    ax,WORD PTR [bp-20]
     16e:	and    ax,WORD PTR [bp-88]
     171:	mov    WORD PTR [bp-60],ax
     174:	mov    cl,0x5
     176:	mov    ax,WORD PTR [bp-18]
     179:	shl    ax,cl
     17b:	add    ax,WORD PTR [bp-36]
     17e:	mov    dx,WORD PTR [bp-60]
     181:	shl    dx,1
     183:	mov    bx,WORD PTR [bp-34]
     186:	mov    WORD PTR [bp-14],bx
     189:	add    ax,dx
     18b:	mov    WORD PTR [bp-16],ax
     18e:	cmp    di,0x100
     192:	jae    1e5 <_LzmaDecode+0x1e5>
     194:	mov    ax,WORD PTR [bp-12]
     197:	movl   dx,ds
     199:	mov    bx,WORD PTR [bp-26]
     19c:	mov    cx,WORD PTR [bp-24]
     19f:	call   1a2 <_LzmaDecode+0x1a2>	1a0: 2	__PTC
     1a2:	je     150 <_LzmaDecode+0x150>
     1a4:	mov    cx,0x8
     1a7:	shl    si,1
     1a9:	rcl    di,1
     1ab:	loop   1a7 <_LzmaDecode+0x1a7>
     1ad:	mov    bx,WORD PTR [bp-12]
     1b0:	mov    al,BYTE PTR [bx]
     1b2:	mov    BYTE PTR [bp-94],al
     1b5:	mov    BYTE PTR [bp-93],0x0
     1b9:	mov    ax,WORD PTR [bp-8]
     1bc:	mov    dx,WORD PTR [bp-4]
     1bf:	mov    cx,0x8
     1c2:	shl    ax,1
     1c4:	rcl    dx,1
     1c6:	loop   1c2 <_LzmaDecode+0x1c2>
     1c8:	mov    bx,WORD PTR [bp-94]
     1cb:	or     bx,ax
     1cd:	mov    WORD PTR [bp-8],bx
     1d0:	mov    WORD PTR [bp-4],dx
     1d3:	mov    ax,WORD PTR [bp-12]
     1d6:	movl   dx,ds
     1d8:	mov    bx,0x1
     1db:	xor    cx,cx
     1dd:	call   1e0 <_LzmaDecode+0x1e0>	1de: 2	__PIA
     1e0:	mov    WORD PTR [bp-12],ax
     1e3:	movl   ds,dx
     1e5:	mov    WORD PTR [bp-102],si
     1e8:	mov    WORD PTR [bp-100],di
     1eb:	mov    cx,0xb
     1ee:	shr    WORD PTR [bp-100],1
     1f1:	rcr    WORD PTR [bp-102],1
     1f4:	loop   1ee <_LzmaDecode+0x1ee>
     1f6:	les    bx,DWORD PTR [bp-16]
     1f9:	mov    bx,WORD PTR es:[bx]
     1fc:	mov    ax,WORD PTR [bp-102]
     1ff:	mov    dx,WORD PTR [bp-100]
     202:	xor    cx,cx
     204:	call   207 <_LzmaDecode+0x207>	205: 2	__U4M
     207:	mov    WORD PTR [bp-10],ax
     20a:	mov    WORD PTR [bp-6],dx
     20d:	mov    ax,WORD PTR [bp-4]
     210:	cmp    ax,dx
     212:	jb     221 <_LzmaDecode+0x221>
     214:	je     219 <_LzmaDecode+0x219>
     216:	jmp    4f7 <_LzmaDecode+0x4f7>
     219:	mov    ax,WORD PTR [bp-8]
     21c:	cmp    ax,WORD PTR [bp-10]
     21f:	jae    216 <_LzmaDecode+0x216>
     221:	mov    WORD PTR [bp-28],0x1
     226:	mov    si,WORD PTR [bp-10]
     229:	mov    di,dx
     22b:	mov    ax,0x800
     22e:	mov    bx,WORD PTR [bp-16]
     231:	sub    ax,WORD PTR es:[bx]
     234:	mov    cl,0x5
     236:	shr    ax,cl
     238:	add    WORD PTR es:[bx],ax
     23b:	mov    cx,0x8
     23e:	sub    cx,WORD PTR [bp-82]
     241:	mov    al,BYTE PTR [bp-2]
     244:	xor    ah,ah
     246:	sar    ax,cl
     248:	cwd
     249:	mov    WORD PTR [bp-94],ax
     24c:	mov    bx,dx
     24e:	mov    ax,WORD PTR [bp-20]
     251:	and    ax,WORD PTR [bp-92]
     254:	mov    dx,WORD PTR [bp-96]
     257:	and    dx,WORD PTR [bp-90]
     25a:	mov    cx,WORD PTR [bp-82]
     25d:	jcxz   265 <_LzmaDecode+0x265>
     25f:	shl    ax,1
     261:	rcl    dx,1
     263:	loop   25f <_LzmaDecode+0x25f>
     265:	add    ax,WORD PTR [bp-94]
     268:	adc    dx,bx
     26a:	mov    bx,0x300
     26d:	xor    cx,cx
     26f:	call   272 <_LzmaDecode+0x272>	270: 2	__U4M
     272:	shl    ax,1
     274:	rcl    dx,1
     276:	mov    dx,WORD PTR [bp-36]
     279:	add    dx,0xe6c
     27d:	mov    bx,WORD PTR [bp-34]
     280:	mov    WORD PTR [bp-14],bx
     283:	add    dx,ax
     285:	mov    WORD PTR [bp-16],dx
     288:	cmp    WORD PTR [bp-18],0x7
     28c:	jge    291 <_LzmaDecode+0x291>
     28e:	jmp    3c0 <_LzmaDecode+0x3c0>
     291:	mov    bx,WORD PTR [bp-20]
     294:	sub    bx,WORD PTR [bp-106]
     297:	mov    cx,WORD PTR [bp-96]
     29a:	sbb    cx,WORD PTR [bp-104]
     29d:	mov    ax,WORD PTR [bp+24]
     2a0:	mov    dx,WORD PTR [bp+26]
     2a3:	call   2a6 <_LzmaDecode+0x2a6>	2a4: 2	__PIA
     2a6:	mov    bx,ax
     2a8:	movl   es,dx
     2aa:	mov    al,BYTE PTR es:[bx]
     2ad:	xor    ah,ah
     2af:	mov    WORD PTR [bp-72],ax
     2b2:	movw    es,WORD PTR [bp-14]
     2b5:	shl    WORD PTR [bp-72],1
     2b8:	mov    ax,WORD PTR [bp-72]
     2bb:	xor    al,al
     2bd:	and    ah,0x1
     2c0:	mov    WORD PTR [bp-68],ax
     2c3:	mov    dx,ax
     2c5:	shl    dx,1
     2c7:	mov    ax,WORD PTR [bp-16]
     2ca:	add    ah,0x2
     2cd:	add    dx,ax
     2cf:	mov    ax,WORD PTR [bp-28]
     2d2:	shl    ax,1
     2d4:	add    dx,ax
     2d6:	mov    WORD PTR [bp-44],dx
     2d9:	cmp    di,0x100
     2dd:	jae    333 <_LzmaDecode+0x333>
     2df:	mov    ax,WORD PTR [bp-12]
     2e2:	movl   dx,ds
     2e4:	mov    bx,WORD PTR [bp-26]
     2e7:	mov    cx,WORD PTR [bp-24]
     2ea:	call   2ed <_LzmaDecode+0x2ed>	2eb: 2	__PTC
     2ed:	jne    2f2 <_LzmaDecode+0x2f2>
     2ef:	jmp    150 <_LzmaDecode+0x150>
     2f2:	mov    cx,0x8
     2f5:	shl    si,1
     2f7:	rcl    di,1
     2f9:	loop   2f5 <_LzmaDecode+0x2f5>
     2fb:	mov    bx,WORD PTR [bp-12]
     2fe:	mov    al,BYTE PTR [bx]
     300:	mov    BYTE PTR [bp-94],al
     303:	mov    BYTE PTR [bp-93],0x0
     307:	mov    ax,WORD PTR [bp-8]
     30a:	mov    dx,WORD PTR [bp-4]
     30d:	mov    cx,0x8
     310:	shl    ax,1
     312:	rcl    dx,1
     314:	loop   310 <_LzmaDecode+0x310>
     316:	mov    bx,WORD PTR [bp-94]
     319:	or     bx,ax
     31b:	mov    WORD PTR [bp-8],bx
     31e:	mov    WORD PTR [bp-4],dx
     321:	mov    ax,WORD PTR [bp-12]
     324:	movl   dx,ds
     326:	mov    bx,0x1
     329:	xor    cx,cx
     32b:	call   32e <_LzmaDecode+0x32e>	32c: 2	__PIA
     32e:	mov    WORD PTR [bp-12],ax
     331:	movl   ds,dx
     333:	mov    WORD PTR [bp-102],si
     336:	mov    WORD PTR [bp-100],di
     339:	mov    cx,0xb
     33c:	shr    WORD PTR [bp-100],1
     33f:	rcr    WORD PTR [bp-102],1
     342:	loop   33c <_LzmaDecode+0x33c>
     344:	mov    bx,WORD PTR [bp-44]
     347:	mov    bx,WORD PTR es:[bx]
     34a:	mov    ax,WORD PTR [bp-102]
     34d:	mov    dx,WORD PTR [bp-100]
     350:	xor    cx,cx
     352:	call   355 <_LzmaDecode+0x355>	353: 2	__U4M
     355:	mov    WORD PTR [bp-10],ax
     358:	mov    WORD PTR [bp-6],dx
     35b:	mov    ax,WORD PTR [bp-4]
     35e:	cmp    ax,dx
     360:	jb     36c <_LzmaDecode+0x36c>
     362:	jne    38c <_LzmaDecode+0x38c>
     364:	mov    ax,WORD PTR [bp-8]
     367:	cmp    ax,WORD PTR [bp-10]
     36a:	jae    38c <_LzmaDecode+0x38c>
     36c:	mov    si,WORD PTR [bp-10]
     36f:	mov    di,dx
     371:	mov    ax,0x800
     374:	mov    bx,WORD PTR [bp-44]
     377:	sub    ax,WORD PTR es:[bx]
     37a:	mov    cl,0x5
     37c:	shr    ax,cl
     37e:	add    WORD PTR es:[bx],ax
     381:	shl    WORD PTR [bp-28],1
     384:	cmp    WORD PTR [bp-68],0x0
     388:	jne    3c0 <_LzmaDecode+0x3c0>
     38a:	jmp    3b6 <_LzmaDecode+0x3b6>
     38c:	sub    si,WORD PTR [bp-10]
     38f:	sbb    di,dx
     391:	mov    ax,WORD PTR [bp-10]
     394:	sub    WORD PTR [bp-8],ax
     397:	sbb    WORD PTR [bp-4],dx
     39a:	mov    cl,0x5
     39c:	mov    bx,WORD PTR [bp-44]
     39f:	mov    ax,WORD PTR es:[bx]
     3a2:	shr    ax,cl
     3a4:	sub    WORD PTR es:[bx],ax
     3a7:	mov    ax,WORD PTR [bp-28]
     3aa:	add    ax,ax
     3ac:	inc    ax
     3ad:	mov    WORD PTR [bp-28],ax
     3b0:	cmp    WORD PTR [bp-68],0x0
     3b4:	je     3c0 <_LzmaDecode+0x3c0>
     3b6:	cmp    WORD PTR [bp-28],0x100
     3bb:	jge    3c0 <_LzmaDecode+0x3c0>
     3bd:	jmp    2b5 <_LzmaDecode+0x2b5>
     3c0:	movw    es,WORD PTR [bp-14]
     3c3:	mov    ax,WORD PTR [bp-28]
     3c6:	cmp    ax,0x100
     3c9:	jl     3ce <_LzmaDecode+0x3ce>
     3cb:	jmp    4ad <_LzmaDecode+0x4ad>
     3ce:	shl    ax,1
     3d0:	mov    bx,WORD PTR [bp-16]
     3d3:	add    bx,ax
     3d5:	mov    WORD PTR [bp-42],bx
     3d8:	cmp    di,0x100
     3dc:	jae    432 <_LzmaDecode+0x432>
     3de:	mov    ax,WORD PTR [bp-12]
     3e1:	movl   dx,ds
     3e3:	mov    bx,WORD PTR [bp-26]
     3e6:	mov    cx,WORD PTR [bp-24]
     3e9:	call   3ec <_LzmaDecode+0x3ec>	3ea: 2	__PTC
     3ec:	jne    3f1 <_LzmaDecode+0x3f1>
     3ee:	jmp    150 <_LzmaDecode+0x150>
     3f1:	mov    cx,0x8
     3f4:	shl    si,1
     3f6:	rcl    di,1
     3f8:	loop   3f4 <_LzmaDecode+0x3f4>
     3fa:	mov    bx,WORD PTR [bp-12]
     3fd:	mov    al,BYTE PTR [bx]
     3ff:	mov    BYTE PTR [bp-94],al
     402:	mov    BYTE PTR [bp-93],0x0
     406:	mov    ax,WORD PTR [bp-8]
     409:	mov    dx,WORD PTR [bp-4]
     40c:	mov    cx,0x8
     40f:	shl    ax,1
     411:	rcl    dx,1
     413:	loop   40f <_LzmaDecode+0x40f>
     415:	mov    bx,WORD PTR [bp-94]
     418:	or     bx,ax
     41a:	mov    WORD PTR [bp-8],bx
     41d:	mov    WORD PTR [bp-4],dx
     420:	mov    ax,WORD PTR [bp-12]
     423:	movl   dx,ds
     425:	mov    bx,0x1
     428:	xor    cx,cx
     42a:	call   42d <_LzmaDecode+0x42d>	42b: 2	__PIA
     42d:	mov    WORD PTR [bp-12],ax
     430:	movl   ds,dx
     432:	mov    WORD PTR [bp-102],si
     435:	mov    WORD PTR [bp-100],di
     438:	mov    cx,0xb
     43b:	shr    WORD PTR [bp-100],1
     43e:	rcr    WORD PTR [bp-102],1
     441:	loop   43b <_LzmaDecode+0x43b>
     443:	mov    bx,WORD PTR [bp-42]
     446:	mov    bx,WORD PTR es:[bx]
     449:	mov    ax,WORD PTR [bp-102]
     44c:	mov    dx,WORD PTR [bp-100]
     44f:	xor    cx,cx
     451:	call   454 <_LzmaDecode+0x454>	452: 2	__U4M
     454:	mov    WORD PTR [bp-10],ax
     457:	mov    WORD PTR [bp-6],dx
     45a:	mov    ax,WORD PTR [bp-4]
     45d:	cmp    ax,dx
     45f:	jb     46b <_LzmaDecode+0x46b>
     461:	jne    486 <_LzmaDecode+0x486>
     463:	mov    ax,WORD PTR [bp-8]
     466:	cmp    ax,WORD PTR [bp-10]
     469:	jae    486 <_LzmaDecode+0x486>
     46b:	mov    si,WORD PTR [bp-10]
     46e:	mov    di,dx
     470:	mov    ax,0x800
     473:	mov    bx,WORD PTR [bp-42]
     476:	sub    ax,WORD PTR es:[bx]
     479:	mov    cl,0x5
     47b:	shr    ax,cl
     47d:	add    WORD PTR es:[bx],ax
     480:	shl    WORD PTR [bp-28],1
     483:	jmp    3c3 <_LzmaDecode+0x3c3>
     486:	sub    si,WORD PTR [bp-10]
     489:	sbb    di,dx
     48b:	mov    ax,WORD PTR [bp-10]
     48e:	sub    WORD PTR [bp-8],ax
     491:	sbb    WORD PTR [bp-4],dx
     494:	mov    cl,0x5
     496:	mov    bx,WORD PTR [bp-42]
     499:	mov    ax,WORD PTR es:[bx]
     49c:	shr    ax,cl
     49e:	sub    WORD PTR es:[bx],ax
     4a1:	mov    ax,WORD PTR [bp-28]
     4a4:	add    ax,ax
     4a6:	inc    ax
     4a7:	mov    WORD PTR [bp-28],ax
     4aa:	jmp    3c3 <_LzmaDecode+0x3c3>
     4ad:	mov    al,BYTE PTR [bp-28]
     4b0:	mov    BYTE PTR [bp-2],al
     4b3:	mov    bx,WORD PTR [bp-20]
     4b6:	mov    cx,WORD PTR [bp-96]
     4b9:	add    WORD PTR [bp-20],0x1
     4bd:	adc    WORD PTR [bp-96],0x0
     4c1:	mov    ax,WORD PTR [bp+24]
     4c4:	mov    dx,WORD PTR [bp+26]
     4c7:	call   4ca <_LzmaDecode+0x4ca>	4c8: 2	__PIA
     4ca:	mov    bx,ax
     4cc:	movl   es,dx
     4ce:	mov    al,BYTE PTR [bp-28]
     4d1:	mov    BYTE PTR es:[bx],al
     4d4:	mov    ax,WORD PTR [bp-18]
     4d7:	cmp    ax,0x4
     4da:	jge    4e4 <_LzmaDecode+0x4e4>
     4dc:	xor    ax,ax
     4de:	mov    WORD PTR [bp-18],ax
     4e1:	jmp    156 <_LzmaDecode+0x156>
     4e4:	cmp    ax,0xa
     4e7:	jge    4f0 <_LzmaDecode+0x4f0>
     4e9:	sub    WORD PTR [bp-18],0x3
     4ed:	jmp    156 <_LzmaDecode+0x156>
     4f0:	sub    WORD PTR [bp-18],0x6
     4f4:	jmp    156 <_LzmaDecode+0x156>
     4f7:	sub    si,WORD PTR [bp-10]
     4fa:	sbb    di,dx
     4fc:	mov    ax,WORD PTR [bp-10]
     4ff:	sub    WORD PTR [bp-8],ax
     502:	sbb    WORD PTR [bp-4],dx
     505:	mov    cl,0x5
     507:	mov    bx,WORD PTR [bp-16]
     50a:	mov    ax,WORD PTR es:[bx]
     50d:	shr    ax,cl
     50f:	sub    WORD PTR es:[bx],ax
     512:	mov    ax,WORD PTR [bp-18]
     515:	shl    ax,1
     517:	mov    dx,WORD PTR [bp-36]
     51a:	add    dx,0x180
     51e:	mov    bx,WORD PTR [bp-34]
     521:	mov    WORD PTR [bp-14],bx
     524:	add    dx,ax
     526:	mov    WORD PTR [bp-16],dx
     529:	cmp    di,0x100
     52d:	jae    583 <_LzmaDecode+0x583>
     52f:	mov    ax,WORD PTR [bp-12]
     532:	movl   dx,ds
     534:	mov    bx,WORD PTR [bp-26]
     537:	mov    cx,WORD PTR [bp-24]
     53a:	call   53d <_LzmaDecode+0x53d>	53b: 2	__PTC
     53d:	jne    542 <_LzmaDecode+0x542>
     53f:	jmp    150 <_LzmaDecode+0x150>
     542:	mov    cx,0x8
     545:	shl    si,1
     547:	rcl    di,1
     549:	loop   545 <_LzmaDecode+0x545>
     54b:	mov    bx,WORD PTR [bp-12]
     54e:	mov    al,BYTE PTR [bx]
     550:	mov    BYTE PTR [bp-94],al
     553:	mov    BYTE PTR [bp-93],0x0
     557:	mov    ax,WORD PTR [bp-8]
     55a:	mov    dx,WORD PTR [bp-4]
     55d:	mov    cx,0x8
     560:	shl    ax,1
     562:	rcl    dx,1
     564:	loop   560 <_LzmaDecode+0x560>
     566:	mov    bx,WORD PTR [bp-94]
     569:	or     bx,ax
     56b:	mov    WORD PTR [bp-8],bx
     56e:	mov    WORD PTR [bp-4],dx
     571:	mov    ax,WORD PTR [bp-12]
     574:	movl   dx,ds
     576:	mov    bx,0x1
     579:	xor    cx,cx
     57b:	call   57e <_LzmaDecode+0x57e>	57c: 2	__PIA
     57e:	mov    WORD PTR [bp-12],ax
     581:	movl   ds,dx
     583:	mov    WORD PTR [bp-102],si
     586:	mov    WORD PTR [bp-100],di
     589:	mov    cx,0xb
     58c:	shr    WORD PTR [bp-100],1
     58f:	rcr    WORD PTR [bp-102],1
     592:	loop   58c <_LzmaDecode+0x58c>
     594:	les    bx,DWORD PTR [bp-16]
     597:	mov    bx,WORD PTR es:[bx]
     59a:	mov    ax,WORD PTR [bp-102]
     59d:	mov    dx,WORD PTR [bp-100]
     5a0:	xor    cx,cx
     5a2:	call   5a5 <_LzmaDecode+0x5a5>	5a3: 2	__U4M
     5a5:	mov    WORD PTR [bp-10],ax
     5a8:	mov    WORD PTR [bp-6],dx
     5ab:	mov    ax,WORD PTR [bp-4]
     5ae:	cmp    ax,dx
     5b0:	jb     5bc <_LzmaDecode+0x5bc>
     5b2:	jne    615 <_LzmaDecode+0x615>
     5b4:	mov    ax,WORD PTR [bp-8]
     5b7:	cmp    ax,WORD PTR [bp-10]
     5ba:	jae    615 <_LzmaDecode+0x615>
     5bc:	mov    si,WORD PTR [bp-10]
     5bf:	mov    di,dx
     5c1:	mov    ax,0x800
     5c4:	mov    bx,WORD PTR [bp-16]
     5c7:	sub    ax,WORD PTR es:[bx]
     5ca:	mov    cl,0x5
     5cc:	shr    ax,cl
     5ce:	add    WORD PTR es:[bx],ax
     5d1:	mov    ax,WORD PTR [bp-56]
     5d4:	mov    WORD PTR [bp-76],ax
     5d7:	mov    ax,WORD PTR [bp-54]
     5da:	mov    WORD PTR [bp-64],ax
     5dd:	mov    ax,WORD PTR [bp-58]
     5e0:	mov    WORD PTR [bp-56],ax
     5e3:	mov    ax,WORD PTR [bp-52]
     5e6:	mov    WORD PTR [bp-54],ax
     5e9:	mov    ax,WORD PTR [bp-106]
     5ec:	mov    WORD PTR [bp-58],ax
     5ef:	mov    ax,WORD PTR [bp-104]
     5f2:	mov    WORD PTR [bp-52],ax
     5f5:	cmp    WORD PTR [bp-18],0x7
     5f9:	jge    5ff <_LzmaDecode+0x5ff>
     5fb:	xor    ax,ax
     5fd:	jmp    602 <_LzmaDecode+0x602>
     5ff:	mov    ax,0x3
     602:	mov    WORD PTR [bp-18],ax
     605:	mov    ax,WORD PTR [bp-34]
     608:	mov    WORD PTR [bp-14],ax
     60b:	mov    bx,WORD PTR [bp-36]
     60e:	add    bx,0x664
     612:	jmp    a6d <_LzmaDecode+0xa6d>
     615:	sub    si,WORD PTR [bp-10]
     618:	sbb    di,dx
     61a:	mov    ax,WORD PTR [bp-10]
     61d:	sub    WORD PTR [bp-8],ax
     620:	sbb    WORD PTR [bp-4],dx
     623:	mov    cl,0x5
     625:	mov    bx,WORD PTR [bp-16]
     628:	mov    ax,WORD PTR es:[bx]
     62b:	shr    ax,cl
     62d:	sub    WORD PTR es:[bx],ax
     630:	mov    dx,WORD PTR [bp-18]
     633:	shl    dx,1
     635:	mov    ax,WORD PTR [bp-36]
     638:	add    ax,0x198
     63b:	mov    bx,WORD PTR [bp-34]
     63e:	mov    WORD PTR [bp-14],bx
     641:	add    ax,dx
     643:	mov    WORD PTR [bp-16],ax
     646:	cmp    di,0x100
     64a:	jae    6a0 <_LzmaDecode+0x6a0>
     64c:	mov    ax,WORD PTR [bp-12]
     64f:	movl   dx,ds
     651:	mov    bx,WORD PTR [bp-26]
     654:	mov    cx,WORD PTR [bp-24]
     657:	call   65a <_LzmaDecode+0x65a>	658: 2	__PTC
     65a:	jne    65f <_LzmaDecode+0x65f>
     65c:	jmp    150 <_LzmaDecode+0x150>
     65f:	mov    cx,0x8
     662:	shl    si,1
     664:	rcl    di,1
     666:	loop   662 <_LzmaDecode+0x662>
     668:	mov    bx,WORD PTR [bp-12]
     66b:	mov    al,BYTE PTR [bx]
     66d:	mov    BYTE PTR [bp-94],al
     670:	mov    BYTE PTR [bp-93],0x0
     674:	mov    ax,WORD PTR [bp-8]
     677:	mov    dx,WORD PTR [bp-4]
     67a:	mov    cx,0x8
     67d:	shl    ax,1
     67f:	rcl    dx,1
     681:	loop   67d <_LzmaDecode+0x67d>
     683:	mov    bx,WORD PTR [bp-94]
     686:	or     bx,ax
     688:	mov    WORD PTR [bp-8],bx
     68b:	mov    WORD PTR [bp-4],dx
     68e:	mov    ax,WORD PTR [bp-12]
     691:	movl   dx,ds
     693:	mov    bx,0x1
     696:	xor    cx,cx
     698:	call   69b <_LzmaDecode+0x69b>	699: 2	__PIA
     69b:	mov    WORD PTR [bp-12],ax
     69e:	movl   ds,dx
     6a0:	mov    WORD PTR [bp-102],si
     6a3:	mov    WORD PTR [bp-100],di
     6a6:	mov    cx,0xb
     6a9:	shr    WORD PTR [bp-100],1
     6ac:	rcr    WORD PTR [bp-102],1
     6af:	loop   6a9 <_LzmaDecode+0x6a9>
     6b1:	les    bx,DWORD PTR [bp-16]
     6b4:	mov    bx,WORD PTR es:[bx]
     6b7:	mov    ax,WORD PTR [bp-102]
     6ba:	mov    dx,WORD PTR [bp-100]
     6bd:	xor    cx,cx
     6bf:	call   6c2 <_LzmaDecode+0x6c2>	6c0: 2	__U4M
     6c2:	mov    WORD PTR [bp-10],ax
     6c5:	mov    WORD PTR [bp-6],dx
     6c8:	mov    ax,WORD PTR [bp-12]
     6cb:	movl   dx,ds
     6cd:	mov    bx,0x1
     6d0:	xor    cx,cx
     6d2:	call   6d5 <_LzmaDecode+0x6d5>	6d3: 2	__PIA
     6d5:	mov    WORD PTR [bp-84],ax
     6d8:	mov    WORD PTR [bp-86],dx
     6db:	mov    ax,WORD PTR [bp-4]
     6de:	cmp    ax,WORD PTR [bp-6]
     6e1:	jb     6f0 <_LzmaDecode+0x6f0>
     6e3:	je     6e8 <_LzmaDecode+0x6e8>
     6e5:	jmp    845 <_LzmaDecode+0x845>
     6e8:	mov    ax,WORD PTR [bp-8]
     6eb:	cmp    ax,WORD PTR [bp-10]
     6ee:	jae    6e5 <_LzmaDecode+0x6e5>
     6f0:	mov    si,WORD PTR [bp-10]
     6f3:	mov    di,WORD PTR [bp-6]
     6f6:	mov    ax,0x800
     6f9:	mov    bx,WORD PTR [bp-16]
     6fc:	sub    ax,WORD PTR es:[bx]
     6ff:	mov    cl,0x5
     701:	shr    ax,cl
     703:	add    WORD PTR es:[bx],ax
     706:	mov    dx,WORD PTR [bp-18]
     709:	shl    dx,cl
     70b:	mov    ax,WORD PTR [bp-36]
     70e:	add    ax,0x1e0
     711:	add    ax,dx
     713:	mov    dx,WORD PTR [bp-60]
     716:	shl    dx,1
     718:	mov    bx,WORD PTR [bp-34]
     71b:	mov    WORD PTR [bp-14],bx
     71e:	add    ax,dx
     720:	mov    WORD PTR [bp-16],ax
     723:	cmp    di,0x100
     727:	jae    778 <_LzmaDecode+0x778>
     729:	mov    ax,WORD PTR [bp-12]
     72c:	movl   dx,ds
     72e:	mov    bx,WORD PTR [bp-26]
     731:	mov    cx,WORD PTR [bp-24]
     734:	call   737 <_LzmaDecode+0x737>	735: 2	__PTC
     737:	jne    73c <_LzmaDecode+0x73c>
     739:	jmp    150 <_LzmaDecode+0x150>
     73c:	mov    ax,si
     73e:	mov    cx,0x8
     741:	shl    ax,1
     743:	rcl    di,1
     745:	loop   741 <_LzmaDecode+0x741>
     747:	mov    si,ax
     749:	mov    bx,WORD PTR [bp-12]
     74c:	mov    al,BYTE PTR [bx]
     74e:	mov    BYTE PTR [bp-94],al
     751:	mov    BYTE PTR [bp-93],0x0
     755:	mov    ax,WORD PTR [bp-8]
     758:	mov    dx,WORD PTR [bp-4]
     75b:	mov    cx,0x8
     75e:	shl    ax,1
     760:	rcl    dx,1
     762:	loop   75e <_LzmaDecode+0x75e>
     764:	mov    bx,WORD PTR [bp-94]
     767:	or     bx,ax
     769:	mov    WORD PTR [bp-8],bx
     76c:	mov    WORD PTR [bp-4],dx
     76f:	mov    bx,WORD PTR [bp-84]
     772:	mov    WORD PTR [bp-12],bx
     775:	movw    ds,WORD PTR [bp-86]
     778:	mov    WORD PTR [bp-102],si
     77b:	mov    WORD PTR [bp-100],di
     77e:	mov    cx,0xb
     781:	shr    WORD PTR [bp-100],1
     784:	rcr    WORD PTR [bp-102],1
     787:	loop   781 <_LzmaDecode+0x781>
     789:	les    bx,DWORD PTR [bp-16]
     78c:	mov    bx,WORD PTR es:[bx]
     78f:	mov    ax,WORD PTR [bp-102]
     792:	mov    dx,WORD PTR [bp-100]
     795:	xor    cx,cx
     797:	call   79a <_LzmaDecode+0x79a>	798: 2	__U4M
     79a:	mov    WORD PTR [bp-10],ax
     79d:	mov    WORD PTR [bp-6],dx
     7a0:	mov    ax,WORD PTR [bp-4]
     7a3:	cmp    ax,dx
     7a5:	jb     7b1 <_LzmaDecode+0x7b1>
     7a7:	jne    7dc <_LzmaDecode+0x7dc>
     7a9:	mov    ax,WORD PTR [bp-8]
     7ac:	cmp    ax,WORD PTR [bp-10]
     7af:	jae    827 <_LzmaDecode+0x827>
     7b1:	mov    si,WORD PTR [bp-10]
     7b4:	mov    di,dx
     7b6:	mov    ax,0x800
     7b9:	mov    bx,WORD PTR [bp-16]
     7bc:	sub    ax,WORD PTR es:[bx]
     7bf:	mov    cl,0x5
     7c1:	shr    ax,cl
     7c3:	add    WORD PTR es:[bx],ax
     7c6:	mov    ax,WORD PTR [bp-96]
     7c9:	or     ax,WORD PTR [bp-20]
     7cc:	jne    7d1 <_LzmaDecode+0x7d1>
     7ce:	jmp    150 <_LzmaDecode+0x150>
     7d1:	cmp    WORD PTR [bp-18],0x7
     7d5:	jge    7de <_LzmaDecode+0x7de>
     7d7:	mov    ax,0x9
     7da:	jmp    7e1 <_LzmaDecode+0x7e1>
     7dc:	jmp    827 <_LzmaDecode+0x827>
     7de:	mov    ax,0xb
     7e1:	mov    WORD PTR [bp-18],ax
     7e4:	mov    bx,WORD PTR [bp-20]
     7e7:	sub    bx,WORD PTR [bp-106]
     7ea:	mov    cx,WORD PTR [bp-96]
     7ed:	sbb    cx,WORD PTR [bp-104]
     7f0:	mov    ax,WORD PTR [bp+24]
     7f3:	mov    dx,WORD PTR [bp+26]
     7f6:	call   7f9 <_LzmaDecode+0x7f9>	7f7: 2	__PIA
     7f9:	mov    bx,ax
     7fb:	movl   es,dx
     7fd:	mov    al,BYTE PTR es:[bx]
     800:	mov    BYTE PTR [bp-2],al
     803:	mov    bx,WORD PTR [bp-20]
     806:	mov    cx,WORD PTR [bp-96]
     809:	add    WORD PTR [bp-20],0x1
     80d:	adc    WORD PTR [bp-96],0x0
     811:	mov    ax,WORD PTR [bp+24]
     814:	mov    dx,WORD PTR [bp+26]
     817:	call   81a <_LzmaDecode+0x81a>	818: 2	__PIA
     81a:	mov    bx,ax
     81c:	movl   es,dx
     81e:	mov    al,BYTE PTR [bp-2]
     821:	mov    BYTE PTR es:[bx],al
     824:	jmp    156 <_LzmaDecode+0x156>
     827:	sub    si,WORD PTR [bp-10]
     82a:	sbb    di,dx
     82c:	mov    ax,WORD PTR [bp-10]
     82f:	sub    WORD PTR [bp-8],ax
     832:	sbb    WORD PTR [bp-4],dx
     835:	mov    cl,0x5
     837:	mov    bx,WORD PTR [bp-16]
     83a:	mov    ax,WORD PTR es:[bx]
     83d:	shr    ax,cl
     83f:	sub    WORD PTR es:[bx],ax
     842:	jmp    a4f <_LzmaDecode+0xa4f>
     845:	sub    si,WORD PTR [bp-10]
     848:	sbb    di,WORD PTR [bp-6]
     84b:	mov    ax,WORD PTR [bp-10]
     84e:	sub    WORD PTR [bp-8],ax
     851:	mov    ax,WORD PTR [bp-6]
     854:	sbb    WORD PTR [bp-4],ax
     857:	mov    cl,0x5
     859:	mov    bx,WORD PTR [bp-16]
     85c:	mov    ax,WORD PTR es:[bx]
     85f:	shr    ax,cl
     861:	sub    WORD PTR es:[bx],ax
     864:	mov    dx,WORD PTR [bp-18]
     867:	shl    dx,1
     869:	mov    ax,WORD PTR [bp-36]
     86c:	add    ax,0x1b0
     86f:	mov    bx,WORD PTR [bp-34]
     872:	mov    WORD PTR [bp-14],bx
     875:	add    ax,dx
     877:	mov    WORD PTR [bp-16],ax
     87a:	cmp    di,0x100
     87e:	jae    8cb <_LzmaDecode+0x8cb>
     880:	mov    ax,WORD PTR [bp-12]
     883:	movl   dx,ds
     885:	mov    bx,WORD PTR [bp-26]
     888:	mov    cx,WORD PTR [bp-24]
     88b:	call   88e <_LzmaDecode+0x88e>	88c: 2	__PTC
     88e:	jne    893 <_LzmaDecode+0x893>
     890:	jmp    150 <_LzmaDecode+0x150>
     893:	mov    cx,0x8
     896:	shl    si,1
     898:	rcl    di,1
     89a:	loop   896 <_LzmaDecode+0x896>
     89c:	mov    bx,WORD PTR [bp-12]
     89f:	mov    al,BYTE PTR [bx]
     8a1:	mov    BYTE PTR [bp-94],al
     8a4:	mov    BYTE PTR [bp-93],0x0
     8a8:	mov    ax,WORD PTR [bp-8]
     8ab:	mov    dx,WORD PTR [bp-4]
     8ae:	mov    cx,0x8
     8b1:	shl    ax,1
     8b3:	rcl    dx,1
     8b5:	loop   8b1 <_LzmaDecode+0x8b1>
     8b7:	mov    bx,WORD PTR [bp-94]
     8ba:	or     bx,ax
     8bc:	mov    WORD PTR [bp-8],bx
     8bf:	mov    WORD PTR [bp-4],dx
     8c2:	mov    bx,WORD PTR [bp-84]
     8c5:	mov    WORD PTR [bp-12],bx
     8c8:	movw    ds,WORD PTR [bp-86]
     8cb:	mov    WORD PTR [bp-102],si
     8ce:	mov    WORD PTR [bp-100],di
     8d1:	mov    cx,0xb
     8d4:	shr    WORD PTR [bp-100],1
     8d7:	rcr    WORD PTR [bp-102],1
     8da:	loop   8d4 <_LzmaDecode+0x8d4>
     8dc:	les    bx,DWORD PTR [bp-16]
     8df:	mov    bx,WORD PTR es:[bx]
     8e2:	mov    ax,WORD PTR [bp-102]
     8e5:	mov    dx,WORD PTR [bp-100]
     8e8:	xor    cx,cx
     8ea:	call   8ed <_LzmaDecode+0x8ed>	8eb: 2	__U4M
     8ed:	mov    WORD PTR [bp-10],ax
     8f0:	mov    WORD PTR [bp-6],dx
     8f3:	mov    ax,WORD PTR [bp-4]
     8f6:	cmp    ax,dx
     8f8:	jb     904 <_LzmaDecode+0x904>
     8fa:	jne    922 <_LzmaDecode+0x922>
     8fc:	mov    ax,WORD PTR [bp-8]
     8ff:	cmp    ax,WORD PTR [bp-10]
     902:	jae    922 <_LzmaDecode+0x922>
     904:	mov    si,WORD PTR [bp-10]
     907:	mov    di,dx
     909:	mov    ax,0x800
     90c:	mov    bx,WORD PTR [bp-16]
     90f:	sub    ax,WORD PTR es:[bx]
     912:	mov    cl,0x5
     914:	shr    ax,cl
     916:	add    WORD PTR es:[bx],ax
     919:	mov    ax,WORD PTR [bp-58]
     91c:	mov    cx,WORD PTR [bp-52]
     91f:	jmp    a3d <_LzmaDecode+0xa3d>
     922:	sub    si,WORD PTR [bp-10]
     925:	sbb    di,dx
     927:	mov    ax,WORD PTR [bp-10]
     92a:	sub    WORD PTR [bp-8],ax
     92d:	sbb    WORD PTR [bp-4],dx
     930:	mov    cl,0x5
     932:	mov    bx,WORD PTR [bp-16]
     935:	mov    ax,WORD PTR es:[bx]
     938:	shr    ax,cl
     93a:	sub    WORD PTR es:[bx],ax
     93d:	mov    ax,WORD PTR [bp-18]
     940:	shl    ax,1
     942:	mov    dx,WORD PTR [bp-36]
     945:	add    dx,0x1c8
     949:	mov    bx,WORD PTR [bp-34]
     94c:	mov    WORD PTR [bp-14],bx
     94f:	add    dx,ax
     951:	mov    WORD PTR [bp-16],dx
     954:	cmp    di,0x100
     958:	jae    9ae <_LzmaDecode+0x9ae>
     95a:	mov    ax,WORD PTR [bp-12]
     95d:	movl   dx,ds
     95f:	mov    bx,WORD PTR [bp-26]
     962:	mov    cx,WORD PTR [bp-24]
     965:	call   968 <_LzmaDecode+0x968>	966: 2	__PTC
     968:	jne    96d <_LzmaDecode+0x96d>
     96a:	jmp    150 <_LzmaDecode+0x150>
     96d:	mov    cx,0x8
     970:	shl    si,1
     972:	rcl    di,1
     974:	loop   970 <_LzmaDecode+0x970>
     976:	mov    bx,WORD PTR [bp-12]
     979:	mov    al,BYTE PTR [bx]
     97b:	mov    BYTE PTR [bp-94],al
     97e:	mov    BYTE PTR [bp-93],0x0
     982:	mov    ax,WORD PTR [bp-8]
     985:	mov    dx,WORD PTR [bp-4]
     988:	mov    cx,0x8
     98b:	shl    ax,1
     98d:	rcl    dx,1
     98f:	loop   98b <_LzmaDecode+0x98b>
     991:	mov    bx,WORD PTR [bp-94]
     994:	or     bx,ax
     996:	mov    WORD PTR [bp-8],bx
     999:	mov    WORD PTR [bp-4],dx
     99c:	mov    ax,WORD PTR [bp-12]
     99f:	movl   dx,ds
     9a1:	mov    bx,0x1
     9a4:	xor    cx,cx
     9a6:	call   9a9 <_LzmaDecode+0x9a9>	9a7: 2	__PIA
     9a9:	mov    WORD PTR [bp-12],ax
     9ac:	movl   ds,dx
     9ae:	mov    WORD PTR [bp-102],si
     9b1:	mov    WORD PTR [bp-100],di
     9b4:	mov    cx,0xb
     9b7:	shr    WORD PTR [bp-100],1
     9ba:	rcr    WORD PTR [bp-102],1
     9bd:	loop   9b7 <_LzmaDecode+0x9b7>
     9bf:	les    bx,DWORD PTR [bp-16]
     9c2:	mov    bx,WORD PTR es:[bx]
     9c5:	mov    ax,WORD PTR [bp-102]
     9c8:	mov    dx,WORD PTR [bp-100]
     9cb:	xor    cx,cx
     9cd:	call   9d0 <_LzmaDecode+0x9d0>	9ce: 2	__U4M
     9d0:	mov    WORD PTR [bp-10],ax
     9d3:	mov    WORD PTR [bp-6],dx
     9d6:	mov    ax,WORD PTR [bp-4]
     9d9:	cmp    ax,dx
     9db:	jb     9e7 <_LzmaDecode+0x9e7>
     9dd:	jne    a04 <_LzmaDecode+0xa04>
     9df:	mov    ax,WORD PTR [bp-8]
     9e2:	cmp    ax,WORD PTR [bp-10]
     9e5:	jae    a04 <_LzmaDecode+0xa04>
     9e7:	mov    si,WORD PTR [bp-10]
     9ea:	mov    di,dx
     9ec:	mov    ax,0x800
     9ef:	mov    bx,WORD PTR [bp-16]
     9f2:	sub    ax,WORD PTR es:[bx]
     9f5:	mov    cl,0x5
     9f7:	shr    ax,cl
     9f9:	add    WORD PTR es:[bx],ax
     9fc:	mov    ax,WORD PTR [bp-56]
     9ff:	mov    cx,WORD PTR [bp-54]
     a02:	jmp    a31 <_LzmaDecode+0xa31>
     a04:	sub    si,WORD PTR [bp-10]
     a07:	sbb    di,dx
     a09:	mov    ax,WORD PTR [bp-10]
     a0c:	sub    WORD PTR [bp-8],ax
     a0f:	sbb    WORD PTR [bp-4],dx
     a12:	mov    cl,0x5
     a14:	mov    bx,WORD PTR [bp-16]
     a17:	mov    ax,WORD PTR es:[bx]
     a1a:	shr    ax,cl
     a1c:	sub    WORD PTR es:[bx],ax
     a1f:	mov    ax,WORD PTR [bp-76]
     a22:	mov    cx,WORD PTR [bp-64]
     a25:	mov    dx,WORD PTR [bp-56]
     a28:	mov    WORD PTR [bp-76],dx
     a2b:	mov    dx,WORD PTR [bp-54]
     a2e:	mov    WORD PTR [bp-64],dx
     a31:	mov    dx,WORD PTR [bp-58]
     a34:	mov    WORD PTR [bp-56],dx
     a37:	mov    dx,WORD PTR [bp-52]
     a3a:	mov    WORD PTR [bp-54],dx
     a3d:	mov    dx,WORD PTR [bp-106]
     a40:	mov    WORD PTR [bp-58],dx
     a43:	mov    dx,WORD PTR [bp-104]
     a46:	mov    WORD PTR [bp-52],dx
     a49:	mov    WORD PTR [bp-106],ax
     a4c:	mov    WORD PTR [bp-104],cx
     a4f:	cmp    WORD PTR [bp-18],0x7
     a53:	jge    a5a <_LzmaDecode+0xa5a>
     a55:	mov    ax,0x8
     a58:	jmp    a5d <_LzmaDecode+0xa5d>
     a5a:	mov    ax,0xb
     a5d:	mov    WORD PTR [bp-18],ax
     a60:	mov    ax,WORD PTR [bp-34]
     a63:	mov    WORD PTR [bp-14],ax
     a66:	mov    bx,WORD PTR [bp-36]
     a69:	add    bx,0xa68
     a6d:	mov    WORD PTR [bp-16],bx
     a70:	mov    bx,WORD PTR [bp-16]
     a73:	mov    WORD PTR [bp-98],bx
     a76:	mov    ax,WORD PTR [bp-14]
     a79:	mov    WORD PTR [bp-30],ax
     a7c:	cmp    di,0x100
     a80:	jae    ad6 <_LzmaDecode+0xad6>
     a82:	mov    ax,WORD PTR [bp-12]
     a85:	movl   dx,ds
     a87:	mov    bx,WORD PTR [bp-26]
     a8a:	mov    cx,WORD PTR [bp-24]
     a8d:	call   a90 <_LzmaDecode+0xa90>	a8e: 2	__PTC
     a90:	jne    a95 <_LzmaDecode+0xa95>
     a92:	jmp    150 <_LzmaDecode+0x150>
     a95:	mov    cx,0x8
     a98:	shl    si,1
     a9a:	rcl    di,1
     a9c:	loop   a98 <_LzmaDecode+0xa98>
     a9e:	mov    bx,WORD PTR [bp-12]
     aa1:	mov    al,BYTE PTR [bx]
     aa3:	mov    BYTE PTR [bp-94],al
     aa6:	mov    BYTE PTR [bp-93],0x0
     aaa:	mov    ax,WORD PTR [bp-8]
     aad:	mov    dx,WORD PTR [bp-4]
     ab0:	mov    cx,0x8
     ab3:	shl    ax,1
     ab5:	rcl    dx,1
     ab7:	loop   ab3 <_LzmaDecode+0xab3>
     ab9:	mov    bx,WORD PTR [bp-94]
     abc:	or     bx,ax
     abe:	mov    WORD PTR [bp-8],bx
     ac1:	mov    WORD PTR [bp-4],dx
     ac4:	mov    ax,WORD PTR [bp-12]
     ac7:	movl   dx,ds
     ac9:	mov    bx,0x1
     acc:	xor    cx,cx
     ace:	call   ad1 <_LzmaDecode+0xad1>	acf: 2	__PIA
     ad1:	mov    WORD PTR [bp-12],ax
     ad4:	movl   ds,dx
     ad6:	mov    WORD PTR [bp-102],si
     ad9:	mov    WORD PTR [bp-100],di
     adc:	mov    cx,0xb
     adf:	shr    WORD PTR [bp-100],1
     ae2:	rcr    WORD PTR [bp-102],1
     ae5:	loop   adf <_LzmaDecode+0xadf>
     ae7:	movw    es,WORD PTR [bp-30]
     aea:	mov    bx,WORD PTR [bp-98]
     aed:	mov    bx,WORD PTR es:[bx]
     af0:	mov    ax,WORD PTR [bp-102]
     af3:	mov    dx,WORD PTR [bp-100]
     af6:	xor    cx,cx
     af8:	call   afb <_LzmaDecode+0xafb>	af9: 2	__U4M
     afb:	mov    WORD PTR [bp-10],ax
     afe:	mov    WORD PTR [bp-6],dx
     b01:	mov    ax,WORD PTR [bp-4]
     b04:	cmp    ax,dx
     b06:	jb     b12 <_LzmaDecode+0xb12>
     b08:	jne    b4c <_LzmaDecode+0xb4c>
     b0a:	mov    ax,WORD PTR [bp-8]
     b0d:	cmp    ax,WORD PTR [bp-10]
     b10:	jae    b4c <_LzmaDecode+0xb4c>
     b12:	mov    si,WORD PTR [bp-10]
     b15:	mov    di,dx
     b17:	mov    ax,0x800
     b1a:	mov    bx,WORD PTR [bp-98]
     b1d:	sub    ax,WORD PTR es:[bx]
     b20:	mov    cl,0x5
     b22:	shr    ax,cl
     b24:	add    WORD PTR es:[bx],ax
     b27:	mov    cl,0x4
     b29:	mov    ax,WORD PTR [bp-60]
     b2c:	shl    ax,cl
     b2e:	mov    dx,WORD PTR [bp-16]
     b31:	add    dx,0x4
     b34:	mov    bx,WORD PTR [bp-14]
     b37:	mov    WORD PTR [bp-30],bx
     b3a:	add    dx,ax
     b3c:	mov    WORD PTR [bp-98],dx
     b3f:	xor    ax,ax
     b41:	mov    WORD PTR [bp-74],ax
     b44:	mov    WORD PTR [bp-62],0x3
     b49:	jmp    c76 <_LzmaDecode+0xc76>
     b4c:	sub    si,WORD PTR [bp-10]
     b4f:	sbb    di,dx
     b51:	mov    ax,WORD PTR [bp-10]
     b54:	sub    WORD PTR [bp-8],ax
     b57:	sbb    WORD PTR [bp-4],dx
     b5a:	mov    cl,0x5
     b5c:	mov    bx,WORD PTR [bp-98]
     b5f:	mov    ax,WORD PTR es:[bx]
     b62:	shr    ax,cl
     b64:	sub    WORD PTR es:[bx],ax
     b67:	mov    ax,WORD PTR [bp-14]
     b6a:	mov    WORD PTR [bp-30],ax
     b6d:	mov    bx,WORD PTR [bp-16]
     b70:	inc    bx
     b71:	inc    bx
     b72:	mov    WORD PTR [bp-98],bx
     b75:	cmp    di,0x100
     b79:	jae    bcf <_LzmaDecode+0xbcf>
     b7b:	mov    ax,WORD PTR [bp-12]
     b7e:	movl   dx,ds
     b80:	mov    bx,WORD PTR [bp-26]
     b83:	mov    cx,WORD PTR [bp-24]
     b86:	call   b89 <_LzmaDecode+0xb89>	b87: 2	__PTC
     b89:	jne    b8e <_LzmaDecode+0xb8e>
     b8b:	jmp    150 <_LzmaDecode+0x150>
     b8e:	mov    cx,0x8
     b91:	shl    si,1
     b93:	rcl    di,1
     b95:	loop   b91 <_LzmaDecode+0xb91>
     b97:	mov    bx,WORD PTR [bp-12]
     b9a:	mov    al,BYTE PTR [bx]
     b9c:	mov    BYTE PTR [bp-94],al
     b9f:	mov    BYTE PTR [bp-93],0x0
     ba3:	mov    ax,WORD PTR [bp-8]
     ba6:	mov    dx,WORD PTR [bp-4]
     ba9:	mov    cx,0x8
     bac:	shl    ax,1
     bae:	rcl    dx,1
     bb0:	loop   bac <_LzmaDecode+0xbac>
     bb2:	mov    bx,WORD PTR [bp-94]
     bb5:	or     bx,ax
     bb7:	mov    WORD PTR [bp-8],bx
     bba:	mov    WORD PTR [bp-4],dx
     bbd:	mov    ax,WORD PTR [bp-12]
     bc0:	movl   dx,ds
     bc2:	mov    bx,0x1
     bc5:	xor    cx,cx
     bc7:	call   bca <_LzmaDecode+0xbca>	bc8: 2	__PIA
     bca:	mov    WORD PTR [bp-12],ax
     bcd:	movl   ds,dx
     bcf:	mov    WORD PTR [bp-102],si
     bd2:	mov    WORD PTR [bp-100],di
     bd5:	mov    cx,0xb
     bd8:	shr    WORD PTR [bp-100],1
     bdb:	rcr    WORD PTR [bp-102],1
     bde:	loop   bd8 <_LzmaDecode+0xbd8>
     be0:	movw    es,WORD PTR [bp-30]
     be3:	mov    bx,WORD PTR [bp-98]
     be6:	mov    bx,WORD PTR es:[bx]
     be9:	mov    ax,WORD PTR [bp-102]
     bec:	mov    dx,WORD PTR [bp-100]
     bef:	xor    cx,cx
     bf1:	call   bf4 <_LzmaDecode+0xbf4>	bf2: 2	__U4M
     bf4:	mov    WORD PTR [bp-10],ax
     bf7:	mov    WORD PTR [bp-6],dx
     bfa:	mov    ax,WORD PTR [bp-4]
     bfd:	cmp    ax,dx
     bff:	jb     c0b <_LzmaDecode+0xc0b>
     c01:	jne    c41 <_LzmaDecode+0xc41>
     c03:	mov    ax,WORD PTR [bp-8]
     c06:	cmp    ax,WORD PTR [bp-10]
     c09:	jae    c41 <_LzmaDecode+0xc41>
     c0b:	mov    si,WORD PTR [bp-10]
     c0e:	mov    di,dx
     c10:	mov    ax,0x800
     c13:	mov    bx,WORD PTR [bp-98]
     c16:	sub    ax,WORD PTR es:[bx]
     c19:	mov    cl,0x5
     c1b:	shr    ax,cl
     c1d:	add    WORD PTR es:[bx],ax
     c20:	mov    cl,0x4
     c22:	mov    ax,WORD PTR [bp-60]
     c25:	shl    ax,cl
     c27:	mov    dx,WORD PTR [bp-16]
     c2a:	add    dx,0x104
     c2e:	mov    bx,WORD PTR [bp-14]
     c31:	mov    WORD PTR [bp-30],bx
     c34:	add    dx,ax
     c36:	mov    WORD PTR [bp-98],dx
     c39:	mov    WORD PTR [bp-74],0x8
     c3e:	jmp    b44 <_LzmaDecode+0xb44>
     c41:	sub    si,WORD PTR [bp-10]
     c44:	sbb    di,dx
     c46:	mov    ax,WORD PTR [bp-10]
     c49:	sub    WORD PTR [bp-8],ax
     c4c:	sbb    WORD PTR [bp-4],dx
     c4f:	mov    cl,0x5
     c51:	mov    bx,WORD PTR [bp-98]
     c54:	mov    ax,WORD PTR es:[bx]
     c57:	shr    ax,cl
     c59:	sub    WORD PTR es:[bx],ax
     c5c:	mov    ax,WORD PTR [bp-14]
     c5f:	mov    WORD PTR [bp-30],ax
     c62:	mov    bx,WORD PTR [bp-16]
     c65:	add    bx,0x204
     c69:	mov    WORD PTR [bp-98],bx
     c6c:	mov    WORD PTR [bp-74],0x10
     c71:	mov    WORD PTR [bp-62],0x8
     c76:	mov    ax,WORD PTR [bp-62]
     c79:	mov    WORD PTR [bp-80],ax
     c7c:	mov    WORD PTR [bp-22],0x1
     c81:	movw    es,WORD PTR [bp-30]
     c84:	mov    ax,WORD PTR [bp-22]
     c87:	shl    ax,1
     c89:	mov    bx,WORD PTR [bp-98]
     c8c:	add    bx,ax
     c8e:	mov    WORD PTR [bp-46],bx
     c91:	cmp    di,0x100
     c95:	jae    ceb <_LzmaDecode+0xceb>
     c97:	mov    ax,WORD PTR [bp-12]
     c9a:	movl   dx,ds
     c9c:	mov    bx,WORD PTR [bp-26]
     c9f:	mov    cx,WORD PTR [bp-24]
     ca2:	call   ca5 <_LzmaDecode+0xca5>	ca3: 2	__PTC
     ca5:	jne    caa <_LzmaDecode+0xcaa>
     ca7:	jmp    150 <_LzmaDecode+0x150>
     caa:	mov    cx,0x8
     cad:	shl    si,1
     caf:	rcl    di,1
     cb1:	loop   cad <_LzmaDecode+0xcad>
     cb3:	mov    bx,WORD PTR [bp-12]
     cb6:	mov    al,BYTE PTR [bx]
     cb8:	mov    BYTE PTR [bp-94],al
     cbb:	mov    BYTE PTR [bp-93],0x0
     cbf:	mov    ax,WORD PTR [bp-8]
     cc2:	mov    dx,WORD PTR [bp-4]
     cc5:	mov    cx,0x8
     cc8:	shl    ax,1
     cca:	rcl    dx,1
     ccc:	loop   cc8 <_LzmaDecode+0xcc8>
     cce:	mov    bx,WORD PTR [bp-94]
     cd1:	or     bx,ax
     cd3:	mov    WORD PTR [bp-8],bx
     cd6:	mov    WORD PTR [bp-4],dx
     cd9:	mov    ax,WORD PTR [bp-12]
     cdc:	movl   dx,ds
     cde:	mov    bx,0x1
     ce1:	xor    cx,cx
     ce3:	call   ce6 <_LzmaDecode+0xce6>	ce4: 2	__PIA
     ce6:	mov    WORD PTR [bp-12],ax
     ce9:	movl   ds,dx
     ceb:	mov    WORD PTR [bp-102],si
     cee:	mov    WORD PTR [bp-100],di
     cf1:	mov    cx,0xb
     cf4:	shr    WORD PTR [bp-100],1
     cf7:	rcr    WORD PTR [bp-102],1
     cfa:	loop   cf4 <_LzmaDecode+0xcf4>
     cfc:	mov    bx,WORD PTR [bp-46]
     cff:	mov    bx,WORD PTR es:[bx]
     d02:	mov    ax,WORD PTR [bp-102]
     d05:	mov    dx,WORD PTR [bp-100]
     d08:	xor    cx,cx
     d0a:	call   d0d <_LzmaDecode+0xd0d>	d0b: 2	__U4M
     d0d:	mov    WORD PTR [bp-10],ax
     d10:	mov    WORD PTR [bp-6],dx
     d13:	mov    ax,WORD PTR [bp-4]
     d16:	cmp    ax,dx
     d18:	jb     d24 <_LzmaDecode+0xd24>
     d1a:	jne    d3e <_LzmaDecode+0xd3e>
     d1c:	mov    ax,WORD PTR [bp-8]
     d1f:	cmp    ax,WORD PTR [bp-10]
     d22:	jae    d3e <_LzmaDecode+0xd3e>
     d24:	mov    si,WORD PTR [bp-10]
     d27:	mov    di,dx
     d29:	mov    ax,0x800
     d2c:	mov    bx,WORD PTR [bp-46]
     d2f:	sub    ax,WORD PTR es:[bx]
     d32:	mov    cl,0x5
     d34:	shr    ax,cl
     d36:	add    WORD PTR es:[bx],ax
     d39:	shl    WORD PTR [bp-22],1
     d3c:	jmp    d62 <_LzmaDecode+0xd62>
     d3e:	sub    si,WORD PTR [bp-10]
     d41:	sbb    di,dx
     d43:	mov    ax,WORD PTR [bp-10]
     d46:	sub    WORD PTR [bp-8],ax
     d49:	sbb    WORD PTR [bp-4],dx
     d4c:	mov    cl,0x5
     d4e:	mov    bx,WORD PTR [bp-46]
     d51:	mov    ax,WORD PTR es:[bx]
     d54:	shr    ax,cl
     d56:	sub    WORD PTR es:[bx],ax
     d59:	mov    ax,WORD PTR [bp-22]
     d5c:	add    ax,ax
     d5e:	inc    ax
     d5f:	mov    WORD PTR [bp-22],ax
     d62:	dec    WORD PTR [bp-80]
     d65:	je     d6a <_LzmaDecode+0xd6a>
     d67:	jmp    c84 <_LzmaDecode+0xc84>
     d6a:	mov    cl,BYTE PTR [bp-62]
     d6d:	mov    ax,0x1
     d70:	shl    ax,cl
     d72:	sub    WORD PTR [bp-22],ax
     d75:	mov    ax,WORD PTR [bp-74]
     d78:	add    WORD PTR [bp-22],ax
     d7b:	cmp    WORD PTR [bp-18],0x4
     d7f:	jl     d84 <_LzmaDecode+0xd84>
     d81:	jmp    10b9 <_LzmaDecode+0x10b9>
     d84:	add    WORD PTR [bp-18],0x7
     d88:	mov    ax,WORD PTR [bp-22]
     d8b:	cmp    ax,0x4
     d8e:	jl     d93 <_LzmaDecode+0xd93>
     d90:	mov    ax,0x3
     d93:	mov    cl,0x7
     d95:	shl    ax,cl
     d97:	mov    dx,WORD PTR [bp-36]
     d9a:	add    dx,0x360
     d9e:	mov    bx,WORD PTR [bp-34]
     da1:	mov    WORD PTR [bp-14],bx
     da4:	add    dx,ax
     da6:	mov    WORD PTR [bp-16],dx
     da9:	mov    WORD PTR [bp-78],0x6
     dae:	mov    WORD PTR [bp-32],0x1
     db3:	movw    es,WORD PTR [bp-14]
     db6:	mov    ax,WORD PTR [bp-32]
     db9:	shl    ax,1
     dbb:	mov    bx,WORD PTR [bp-16]
     dbe:	add    bx,ax
     dc0:	mov    WORD PTR [bp-40],bx
     dc3:	cmp    di,0x100
     dc7:	jae    e1d <_LzmaDecode+0xe1d>
     dc9:	mov    ax,WORD PTR [bp-12]
     dcc:	movl   dx,ds
     dce:	mov    bx,WORD PTR [bp-26]
     dd1:	mov    cx,WORD PTR [bp-24]
     dd4:	call   dd7 <_LzmaDecode+0xdd7>	dd5: 2	__PTC
     dd7:	jne    ddc <_LzmaDecode+0xddc>
     dd9:	jmp    150 <_LzmaDecode+0x150>
     ddc:	mov    cx,0x8
     ddf:	shl    si,1
     de1:	rcl    di,1
     de3:	loop   ddf <_LzmaDecode+0xddf>
     de5:	mov    bx,WORD PTR [bp-12]
     de8:	mov    al,BYTE PTR [bx]
     dea:	mov    BYTE PTR [bp-94],al
     ded:	mov    BYTE PTR [bp-93],0x0
     df1:	mov    ax,WORD PTR [bp-8]
     df4:	mov    dx,WORD PTR [bp-4]
     df7:	mov    cx,0x8
     dfa:	shl    ax,1
     dfc:	rcl    dx,1
     dfe:	loop   dfa <_LzmaDecode+0xdfa>
     e00:	mov    bx,WORD PTR [bp-94]
     e03:	or     bx,ax
     e05:	mov    WORD PTR [bp-8],bx
     e08:	mov    WORD PTR [bp-4],dx
     e0b:	mov    ax,WORD PTR [bp-12]
     e0e:	movl   dx,ds
     e10:	mov    bx,0x1
     e13:	xor    cx,cx
     e15:	call   e18 <_LzmaDecode+0xe18>	e16: 2	__PIA
     e18:	mov    WORD PTR [bp-12],ax
     e1b:	movl   ds,dx
     e1d:	mov    WORD PTR [bp-102],si
     e20:	mov    WORD PTR [bp-100],di
     e23:	mov    cx,0xb
     e26:	shr    WORD PTR [bp-100],1
     e29:	rcr    WORD PTR [bp-102],1
     e2c:	loop   e26 <_LzmaDecode+0xe26>
     e2e:	mov    bx,WORD PTR [bp-40]
     e31:	mov    bx,WORD PTR es:[bx]
     e34:	mov    ax,WORD PTR [bp-102]
     e37:	mov    dx,WORD PTR [bp-100]
     e3a:	xor    cx,cx
     e3c:	call   e3f <_LzmaDecode+0xe3f>	e3d: 2	__U4M
     e3f:	mov    WORD PTR [bp-10],ax
     e42:	mov    WORD PTR [bp-6],dx
     e45:	mov    ax,WORD PTR [bp-4]
     e48:	cmp    ax,dx
     e4a:	jb     e56 <_LzmaDecode+0xe56>
     e4c:	jne    e70 <_LzmaDecode+0xe70>
     e4e:	mov    ax,WORD PTR [bp-8]
     e51:	cmp    ax,WORD PTR [bp-10]
     e54:	jae    e70 <_LzmaDecode+0xe70>
     e56:	mov    si,WORD PTR [bp-10]
     e59:	mov    di,dx
     e5b:	mov    ax,0x800
     e5e:	mov    bx,WORD PTR [bp-40]
     e61:	sub    ax,WORD PTR es:[bx]
     e64:	mov    cl,0x5
     e66:	shr    ax,cl
     e68:	add    WORD PTR es:[bx],ax
     e6b:	shl    WORD PTR [bp-32],1
     e6e:	jmp    e94 <_LzmaDecode+0xe94>
     e70:	sub    si,WORD PTR [bp-10]
     e73:	sbb    di,dx
     e75:	mov    ax,WORD PTR [bp-10]
     e78:	sub    WORD PTR [bp-8],ax
     e7b:	sbb    WORD PTR [bp-4],dx
     e7e:	mov    cl,0x5
     e80:	mov    bx,WORD PTR [bp-40]
     e83:	mov    ax,WORD PTR es:[bx]
     e86:	shr    ax,cl
     e88:	sub    WORD PTR es:[bx],ax
     e8b:	mov    ax,WORD PTR [bp-32]
     e8e:	add    ax,ax
     e90:	inc    ax
     e91:	mov    WORD PTR [bp-32],ax
     e94:	dec    WORD PTR [bp-78]
     e97:	je     e9c <_LzmaDecode+0xe9c>
     e99:	jmp    db6 <_LzmaDecode+0xdb6>
     e9c:	sub    WORD PTR [bp-32],0x40
     ea0:	mov    ax,WORD PTR [bp-32]
     ea3:	cmp    ax,0x4
     ea6:	jl     ef3 <_LzmaDecode+0xef3>
     ea8:	sar    ax,1
     eaa:	dec    ax
     eab:	mov    WORD PTR [bp-38],ax
     eae:	mov    ax,WORD PTR [bp-32]
     eb1:	and    ax,0x1
     eb4:	or     al,0x2
     eb6:	mov    WORD PTR [bp-106],ax
     eb9:	xor    ax,ax
     ebb:	mov    WORD PTR [bp-104],ax
     ebe:	cmp    WORD PTR [bp-32],0xe
     ec2:	jge    ef6 <_LzmaDecode+0xef6>
     ec4:	mov    cx,WORD PTR [bp-38]
     ec7:	jcxz   ed1 <_LzmaDecode+0xed1>
     ec9:	shl    WORD PTR [bp-106],1
     ecc:	rcl    WORD PTR [bp-104],1
     ecf:	loop   ec9 <_LzmaDecode+0xec9>
     ed1:	mov    dx,WORD PTR [bp-106]
     ed4:	shl    dx,1
     ed6:	mov    ax,WORD PTR [bp-36]
     ed9:	add    ax,0x560
     edc:	add    dx,ax
     ede:	mov    ax,WORD PTR [bp-32]
     ee1:	shl    ax,1
     ee3:	sub    dx,ax
     ee5:	mov    ax,WORD PTR [bp-34]
     ee8:	mov    WORD PTR [bp-14],ax
     eeb:	dec    dx
     eec:	dec    dx
     eed:	mov    WORD PTR [bp-16],dx
     ef0:	jmp    f9e <_LzmaDecode+0xf9e>
     ef3:	jmp    109f <_LzmaDecode+0x109f>
     ef6:	sub    WORD PTR [bp-38],0x4
     efa:	cmp    di,0x100
     efe:	jae    f54 <_LzmaDecode+0xf54>
     f00:	mov    ax,WORD PTR [bp-12]
     f03:	movl   dx,ds
     f05:	mov    bx,WORD PTR [bp-26]
     f08:	mov    cx,WORD PTR [bp-24]
     f0b:	call   f0e <_LzmaDecode+0xf0e>	f0c: 2	__PTC
     f0e:	jne    f13 <_LzmaDecode+0xf13>
     f10:	jmp    150 <_LzmaDecode+0x150>
     f13:	mov    cx,0x8
     f16:	shl    si,1
     f18:	rcl    di,1
     f1a:	loop   f16 <_LzmaDecode+0xf16>
     f1c:	mov    bx,WORD PTR [bp-12]
     f1f:	mov    al,BYTE PTR [bx]
     f21:	mov    BYTE PTR [bp-94],al
     f24:	mov    BYTE PTR [bp-93],0x0
     f28:	mov    ax,WORD PTR [bp-8]
     f2b:	mov    dx,WORD PTR [bp-4]
     f2e:	mov    cx,0x8
     f31:	shl    ax,1
     f33:	rcl    dx,1
     f35:	loop   f31 <_LzmaDecode+0xf31>
     f37:	mov    bx,WORD PTR [bp-94]
     f3a:	or     bx,ax
     f3c:	mov    WORD PTR [bp-8],bx
     f3f:	mov    WORD PTR [bp-4],dx
     f42:	mov    ax,WORD PTR [bp-12]
     f45:	movl   dx,ds
     f47:	mov    bx,0x1
     f4a:	xor    cx,cx
     f4c:	call   f4f <_LzmaDecode+0xf4f>	f4d: 2	__PIA
     f4f:	mov    WORD PTR [bp-12],ax
     f52:	movl   ds,dx
     f54:	shr    di,1
     f56:	rcr    si,1
     f58:	shl    WORD PTR [bp-106],1
     f5b:	rcl    WORD PTR [bp-104],1
     f5e:	mov    ax,WORD PTR [bp-4]
     f61:	cmp    di,ax
     f63:	jb     f6c <_LzmaDecode+0xf6c>
     f65:	jne    f76 <_LzmaDecode+0xf76>
     f67:	cmp    si,WORD PTR [bp-8]
     f6a:	ja     f76 <_LzmaDecode+0xf76>
     f6c:	sub    WORD PTR [bp-8],si
     f6f:	sbb    WORD PTR [bp-4],di
     f72:	or     BYTE PTR [bp-106],0x1
     f76:	dec    WORD PTR [bp-38]
     f79:	je     f7e <_LzmaDecode+0xf7e>
     f7b:	jmp    efa <_LzmaDecode+0xefa>
     f7e:	mov    ax,WORD PTR [bp-34]
     f81:	mov    WORD PTR [bp-14],ax
     f84:	mov    bx,WORD PTR [bp-36]
     f87:	add    bx,0x644
     f8b:	mov    WORD PTR [bp-16],bx
     f8e:	mov    cx,0x4
     f91:	shl    WORD PTR [bp-106],1
     f94:	rcl    WORD PTR [bp-104],1
     f97:	loop   f91 <_LzmaDecode+0xf91>
     f99:	mov    WORD PTR [bp-38],0x4
     f9e:	mov    ax,0x1
     fa1:	mov    WORD PTR [bp-70],ax
     fa4:	mov    WORD PTR [bp-48],ax
     fa7:	movw    es,WORD PTR [bp-14]
     faa:	mov    ax,WORD PTR [bp-48]
     fad:	shl    ax,1
     faf:	mov    bx,WORD PTR [bp-16]
     fb2:	add    bx,ax
     fb4:	mov    WORD PTR [bp-50],bx
     fb7:	cmp    di,0x100
     fbb:	jae    1011 <_LzmaDecode+0x1011>
     fbd:	mov    ax,WORD PTR [bp-12]
     fc0:	movl   dx,ds
     fc2:	mov    bx,WORD PTR [bp-26]
     fc5:	mov    cx,WORD PTR [bp-24]
     fc8:	call   fcb <_LzmaDecode+0xfcb>	fc9: 2	__PTC
     fcb:	jne    fd0 <_LzmaDecode+0xfd0>
     fcd:	jmp    150 <_LzmaDecode+0x150>
     fd0:	mov    cx,0x8
     fd3:	shl    si,1
     fd5:	rcl    di,1
     fd7:	loop   fd3 <_LzmaDecode+0xfd3>
     fd9:	mov    bx,WORD PTR [bp-12]
     fdc:	mov    al,BYTE PTR [bx]
     fde:	mov    BYTE PTR [bp-94],al
     fe1:	mov    BYTE PTR [bp-93],0x0
     fe5:	mov    ax,WORD PTR [bp-8]
     fe8:	mov    dx,WORD PTR [bp-4]
     feb:	mov    cx,0x8
     fee:	shl    ax,1
     ff0:	rcl    dx,1
     ff2:	loop   fee <_LzmaDecode+0xfee>
     ff4:	mov    bx,WORD PTR [bp-94]
     ff7:	or     bx,ax
     ff9:	mov    WORD PTR [bp-8],bx
     ffc:	mov    WORD PTR [bp-4],dx
     fff:	mov    ax,WORD PTR [bp-12]
    1002:	movl   dx,ds
    1004:	mov    bx,0x1
    1007:	xor    cx,cx
    1009:	call   100c <_LzmaDecode+0x100c>	100a: 2	__PIA
    100c:	mov    WORD PTR [bp-12],ax
    100f:	movl   ds,dx
    1011:	mov    WORD PTR [bp-102],si
    1014:	mov    WORD PTR [bp-100],di
    1017:	mov    cx,0xb
    101a:	shr    WORD PTR [bp-100],1
    101d:	rcr    WORD PTR [bp-102],1
    1020:	loop   101a <_LzmaDecode+0x101a>
    1022:	mov    bx,WORD PTR [bp-50]
    1025:	mov    bx,WORD PTR es:[bx]
    1028:	mov    ax,WORD PTR [bp-102]
    102b:	mov    dx,WORD PTR [bp-100]
    102e:	xor    cx,cx
    1030:	call   1033 <_LzmaDecode+0x1033>	1031: 2	__U4M
    1033:	mov    WORD PTR [bp-10],ax
    1036:	mov    WORD PTR [bp-6],dx
    1039:	mov    ax,WORD PTR [bp-4]
    103c:	cmp    ax,dx
    103e:	jb     104a <_LzmaDecode+0x104a>
    1040:	jne    1064 <_LzmaDecode+0x1064>
    1042:	mov    ax,WORD PTR [bp-8]
    1045:	cmp    ax,WORD PTR [bp-10]
    1048:	jae    1064 <_LzmaDecode+0x1064>
    104a:	mov    si,WORD PTR [bp-10]
    104d:	mov    di,dx
    104f:	mov    ax,0x800
    1052:	mov    bx,WORD PTR [bp-50]
    1055:	sub    ax,WORD PTR es:[bx]
    1058:	mov    cl,0x5
    105a:	shr    ax,cl
    105c:	add    WORD PTR es:[bx],ax
    105f:	shl    WORD PTR [bp-48],1
    1062:	jmp    1092 <_LzmaDecode+0x1092>
    1064:	sub    si,WORD PTR [bp-10]
    1067:	sbb    di,dx
    1069:	mov    ax,WORD PTR [bp-10]
    106c:	sub    WORD PTR [bp-8],ax
    106f:	sbb    WORD PTR [bp-4],dx
    1072:	mov    cl,0x5
    1074:	mov    bx,WORD PTR [bp-50]
    1077:	mov    ax,WORD PTR es:[bx]
    107a:	shr    ax,cl
    107c:	sub    WORD PTR es:[bx],ax
    107f:	mov    ax,WORD PTR [bp-48]
    1082:	add    ax,ax
    1084:	inc    ax
    1085:	mov    WORD PTR [bp-48],ax
    1088:	mov    ax,WORD PTR [bp-70]
    108b:	cwd
    108c:	or     WORD PTR [bp-106],ax
    108f:	or     WORD PTR [bp-104],dx
    1092:	shl    WORD PTR [bp-70],1
    1095:	dec    WORD PTR [bp-38]
    1098:	je     109d <_LzmaDecode+0x109d>
    109a:	jmp    faa <_LzmaDecode+0xfaa>
    109d:	jmp    10a6 <_LzmaDecode+0x10a6>
    109f:	cwd
    10a0:	mov    WORD PTR [bp-106],ax
    10a3:	mov    WORD PTR [bp-104],dx
    10a6:	add    WORD PTR [bp-106],0x1
    10aa:	adc    WORD PTR [bp-104],0x0
    10ae:	mov    ax,WORD PTR [bp-104]
    10b1:	or     ax,WORD PTR [bp-106]
    10b4:	jne    10b9 <_LzmaDecode+0x10b9>
    10b6:	jmp    1132 <_LzmaDecode+0x1132>
    10b9:	add    WORD PTR [bp-22],0x2
    10bd:	mov    ax,WORD PTR [bp-104]
    10c0:	cmp    ax,WORD PTR [bp-96]
    10c3:	jbe    10c8 <_LzmaDecode+0x10c8>
    10c5:	jmp    150 <_LzmaDecode+0x150>
    10c8:	jne    10d2 <_LzmaDecode+0x10d2>
    10ca:	mov    ax,WORD PTR [bp-106]
    10cd:	cmp    ax,WORD PTR [bp-20]
    10d0:	ja     10c5 <_LzmaDecode+0x10c5>
    10d2:	mov    bx,WORD PTR [bp-20]
    10d5:	sub    bx,WORD PTR [bp-106]
    10d8:	mov    cx,WORD PTR [bp-96]
    10db:	sbb    cx,WORD PTR [bp-104]
    10de:	mov    ax,WORD PTR [bp+24]
    10e1:	mov    dx,WORD PTR [bp+26]
    10e4:	call   10e7 <_LzmaDecode+0x10e7>	10e5: 2	__PIA
    10e7:	mov    bx,ax
    10e9:	movl   es,dx
    10eb:	mov    al,BYTE PTR es:[bx]
    10ee:	mov    BYTE PTR [bp-2],al
    10f1:	mov    bx,WORD PTR [bp-20]
    10f4:	mov    cx,WORD PTR [bp-96]
    10f7:	dec    WORD PTR [bp-22]
    10fa:	add    WORD PTR [bp-20],0x1
    10fe:	adc    WORD PTR [bp-96],0x0
    1102:	mov    ax,WORD PTR [bp+24]
    1105:	mov    dx,WORD PTR [bp+26]
    1108:	call   110b <_LzmaDecode+0x110b>	1109: 2	__PIA
    110b:	mov    bx,ax
    110d:	movl   es,dx
    110f:	mov    al,BYTE PTR [bp-2]
    1112:	mov    BYTE PTR es:[bx],al
    1115:	cmp    WORD PTR [bp-22],0x0
    1119:	jne    111e <_LzmaDecode+0x111e>
    111b:	jmp    156 <_LzmaDecode+0x156>
    111e:	mov    ax,WORD PTR [bp-96]
    1121:	cmp    ax,WORD PTR [bp+30]
    1124:	jb     10d2 <_LzmaDecode+0x10d2>
    1126:	jne    111b <_LzmaDecode+0x111b>
    1128:	mov    ax,WORD PTR [bp-20]
    112b:	cmp    ax,WORD PTR [bp+28]
    112e:	jb     10d2 <_LzmaDecode+0x10d2>
    1130:	jmp    111b <_LzmaDecode+0x111b>
    1132:	cmp    di,0x100
    1136:	jae    115d <_LzmaDecode+0x115d>
    1138:	mov    ax,WORD PTR [bp-12]
    113b:	movl   dx,ds
    113d:	mov    bx,WORD PTR [bp-26]
    1140:	mov    cx,WORD PTR [bp-24]
    1143:	call   1146 <_LzmaDecode+0x1146>	1144: 2	__PTC
    1146:	jne    114b <_LzmaDecode+0x114b>
    1148:	jmp    150 <_LzmaDecode+0x150>
    114b:	mov    ax,WORD PTR [bp-12]
    114e:	movl   dx,ds
    1150:	mov    bx,0x1
    1153:	xor    cx,cx
    1155:	call   1158 <_LzmaDecode+0x1158>	1156: 2	__PIA
    1158:	mov    WORD PTR [bp-12],ax
    115b:	movl   ds,dx
    115d:	mov    ax,WORD PTR [bp-12]
    1160:	movl   dx,ds
    1162:	mov    bx,WORD PTR [bp+12]
    1165:	mov    cx,WORD PTR [bp+14]
    1168:	call   116b <_LzmaDecode+0x116b>	1169: 2	__PTS
    116b:	lds    bx,DWORD PTR [bp+20]
    116e:	mov    WORD PTR [bx],ax
    1170:	mov    WORD PTR [bx+2],dx
    1173:	mov    ax,WORD PTR [bp-20]
    1176:	lds    bx,DWORD PTR [bp+32]
    1179:	mov    WORD PTR [bx],ax
    117b:	mov    ax,WORD PTR [bp-96]
    117e:	mov    WORD PTR [bx+2],ax
    1181:	xor    ax,ax
    1183:	mov    sp,bp
    1185:	pop    bp
    1186:	pop    di
    1187:	pop    si
    1188:	ret
Disassembly of section .text:

0000000000000000 <_LzmaDecodeProperties>:
   0:	push   si
   1:	push   bp
   2:	mov    bp,sp
   4:	lds    bx,DWORD PTR [bp+6]
   7:	cmp    WORD PTR [bp+14],0x5
   b:	jge    13 <_LzmaDecodeProperties+0x13>
   d:	mov    ax,0x1
  10:	pop    bp
  11:	pop    si
  12:	ret
  13:	les    si,DWORD PTR [bp+10]
  16:	mov    al,BYTE PTR es:[si]
  19:	cmp    al,0xe1
  1b:	jae    d <_LzmaDecodeProperties+0xd>
  1d:	mov    WORD PTR [bx+4],0x0
  22:	cmp    al,0x2d
  24:	jb     2d <_LzmaDecodeProperties+0x2d>
  26:	inc    WORD PTR [bx+4]
  29:	sub    al,0x2d
  2b:	jmp    22 <_LzmaDecodeProperties+0x22>
  2d:	mov    WORD PTR [bx+2],0x0
  32:	cmp    al,0x9
  34:	jb     3d <_LzmaDecodeProperties+0x3d>
  36:	inc    WORD PTR [bx+2]
  39:	sub    al,0x9
  3b:	jmp    32 <_LzmaDecodeProperties+0x32>
  3d:	xor    ah,ah
  3f:	mov    WORD PTR [bx],ax
  41:	xor    al,al
  43:	pop    bp
  44:	pop    si
  45:	ret
