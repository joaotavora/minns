// libc includes
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

// stdlib includes
#include <iostream>
#include <string>
#include <sstream>
#include <string>

// Project includes
#include "helper.h"

void hexdump(void *pAddressIn, long  lSize)
{
 char szBuf[100];
 long lIndent = 1;
 long lOutLen, lIndex, lIndex2, lOutLen2;
 long lRelPos;
 struct { char *pData; unsigned long lSize; } buf;
 unsigned char *pTmp,ucTmp;
 unsigned char *pAddress = (unsigned char *)pAddressIn;

   buf.pData   = (char *)pAddress;
   buf.lSize   = lSize;

   while (buf.lSize > 0)
   {
      pTmp     = (unsigned char *)buf.pData;
      lOutLen  = (int)buf.lSize;
      if (lOutLen > 16)
          lOutLen = 16;

      // create a 64-character formatted output line:
      sprintf(szBuf, " >                            "
                     "                      "
                     "    %08X", pTmp-pAddress);
      lOutLen2 = lOutLen;

      for(lIndex = 1+lIndent, lIndex2 = 53-15+lIndent, lRelPos = 0;
          lOutLen2;
          lOutLen2--, lIndex += 2, lIndex2++
         )
      {
         ucTmp = *pTmp++;

         sprintf(szBuf + lIndex, "%02X ", (unsigned short)ucTmp);
         if(!isprint(ucTmp))  ucTmp = '.'; // nonprintable char
         szBuf[lIndex2] = ucTmp;

         if (!(++lRelPos & 3))     // extra blank after 4 bytes
         {  lIndex++; szBuf[lIndex+2] = ' '; }
      }

      if (!(lRelPos & 3)) lIndex--;

      szBuf[lIndex  ]   = '<';
      szBuf[lIndex+1]   = ' ';

      printf("%s\n", szBuf);

      buf.pData   += lOutLen;
      buf.lSize   -= lOutLen;
   }
}

unsigned int strtol_helper(char c, char* arg, unsigned int const* defaults) throw (std::runtime_error){
    static std::stringstream ss;
    char *endptr;
    long val;

    errno = 0;
    val = strtol(arg, &endptr, 0);
    
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0)) {
        ss << "Error reading value of -" << c << ": " << strerror(errno) << ". Using default value";
        throw std::runtime_error(ss.str().c_str());
    }
    
    if (endptr == arg) {
        ss << "Non numeric value of -" << c << " . Using default value";
        throw std::runtime_error(ss.str().c_str());
    }

    if (*endptr != '\0')        /* Not necessarily an error... */
        std::cerr << "   (Further characters after number ignored: \'" << endptr << "\')" << std::endl;

    if (val > (long)defaults[1]) {
        ss << "Value for -" << c << " is too big: (maximum is " << defaults[1] << ")";
        throw std::runtime_error(ss.str().c_str());
    }

    if (val < (long)defaults[0]) {
        ss << "Value for -" << c << " is too small: (minimum is " << defaults[0] << ")";
        throw std::runtime_error(ss.str().c_str());
    }
        
    return (unsigned int)val;
}


