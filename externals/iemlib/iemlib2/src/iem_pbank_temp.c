// 1.)  allow only readable ASCII (0x0a, 0x0d, 0x20...0x7e), else drop
    txvec_src = txbuf1;
    txvec_dst = txbuf2;
	txlen1 = txalloc;
    p = 0;
    for(l=0; l<txlen; l++)
    {
      if(((*txvec_src >= ' ') && (*txvec_src <= '~')) || (*txvec_src == 0x0a) || (*txvec_src == 0x0d))
      {
        *txvec_dst++ = *txvec_src++;// copy the same char
        p++;
      }
      else
        txvec_src++;// drop anything else
    }
    txlen = p - 1; // dst is 2
	
	// 2.) unify windows return
    txvec_src = txbuf2;
    txvec_dst = txbuf1;
    p = 0;
    for(l=0; l<txlen; l++)
    {
      if((*txvec_src == 0x0d)&&(txvec_src[1] == 0x0a))  // windows return
      {
        *txvec_dst++ = IEM_PBANK_UNIFIED_EOL;
        txvec_src += 2;
        l++;
        p++;
      }
      else
      {
        *txvec_dst++ = *txvec_src++;
        p++;
      }
    }
    txlen = p; // dst is 1

	
	
    IEM_PBANK_UNIFIED_EOL  IEM_PBANK_UNIFIED_SEP
     
    
    
    // unify return
    txvec_dst = txbuf2;
    for(l=0; l<txlen; l++)
    {
      if(*txvec_dst == 0x0d)// replace '0x0d' by '0x0a'
        *txvec_dst = 0x0a;
      txvec_dst++;
    }
    
    // unify EndOfLine
    txvec_src = txbuf2;
    txvec_dst = txbuf1;
    p = 0;
    for(l=0; l<txlen; l++)
    {
      if(!strncmp(txvec_src, eol+eol_offset, eol_length)) /* replace eol by 0x0a */
      {
        txvec_src += eol_length;
        l += eol_length - 1;
        *txvec_dst++ = 0x0a;
        p++;
      }
      else
      {
        *txvec_dst++ = *txvec_src++;// copy the same char
        p++;
      }
    }
    txlen = p;
    
#if(IEMLIB2_DEBUG)
    post("after UNI EOL RET");
    txvec_src = txbuf1;
    for(l=0; l<txlen; l++)
    {
      post("%d:%x=%c",l,*txvec_src,*txvec_src);
      txvec_src++;
    }
#endif
    
    // unify separator
    txvec_src = txbuf1;
    txvec_dst = txbuf2;
    for(l=0; l<txlen; l++)
    {
      if(*txvec_src == sep)// replace 'sep' by ';'
      {
        txvec_src++;
        *txvec_dst++ = ';';
      }
      else if((*txvec_src == ',') && (mode[IEM_PBANK_END_OF_LINE] != 'c')) // replace ',' by '.'
      {
        txvec_src++;
        *txvec_dst++ = '.';
      }
      else
        *txvec_dst++ = *txvec_src++;// drop anything else
    }