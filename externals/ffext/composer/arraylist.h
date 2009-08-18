/* ------------------------------------------------------------------------ */
/* Copyright (c) 2009 Federico Ferri.                                       */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL */
/* WARRANTIES, see the file, "LICENSE.txt," in this distribution.           */
/*                                                                          */
/* composer: a music composition framework for pure-data                    */
/*                                                                          */
/* This program is free software; you can redistribute it and/or            */
/* modify it under the terms of the GNU General Public License              */
/* as published by the Free Software Foundation; either version 2           */
/* of the License, or (at your option) any later version.                   */
/*                                                                          */
/* See file LICENSE for further informations on licensing terms.            */
/*                                                                          */
/* This program is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/* GNU General Public License for more details.                             */
/*                                                                          */
/* You should have received a copy of the GNU General Public License        */
/* along with this program; if not, write to the Free Software Foundation,  */
/* Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.          */
/*                                                                          */
/* Based on PureData by Miller Puckette and others.                         */
/* ------------------------------------------------------------------------ */

#include "common.h"

#ifndef __ARRAYLIST_H_INCLUDED_
#define __ARRAYLIST_H_INCLUDED_

#define ArrayListDeclare(name, type, sizetype) \
    type* name; \
    sizetype name ## _maxsize; \
    sizetype name ## _count

#define ArrayListDeclareWithPrefix(prefix, name, type, sizetype) \
    prefix type* name; \
    prefix sizetype name ## _maxsize; \
    prefix sizetype name ## _count

#define ArrayListInit(arrName, type, initSize) \
    arrName ## _maxsize = initSize; \
    arrName = (type*)getbytes(sizeof(type) * (initSize)); \
    arrName ## _count = 0

#define ArrayListAdd(arrName, type, objToAdd) \
    if(arrName ## _count >= arrName ## _maxsize) { \
        arrName = (type*)resizebytes(arrName, arrName ## _maxsize, (arrName ## _maxsize)*2); \
    }; \
    arrName[ arrName ## _count ++ ] = (type) objToAdd; \

#define ArrayListRemove(arrName, objToRem) \
    { \
        int i,j; \
        for(i=0; i< arrName ## _count; i++) \
            if(arrName[i] == objToRem) { \
                for(j=i; j< arrName ## _count - 1; j++) { \
                    arrName[j] =  arrName[j+1]; \
                } \
                arrName[ arrName ## _count -- ] = 0L; \
                break; \
            } \
    }

#define ArrayListRemoveByIndex(arrName, index) \
    { \
        int i,j; \
        for(i=0; i< arrName ## _count; i++) \
            if(i == index) { \
                for(j=i; j< arrName ## _count - 1; j++) { \
                    arrName[j] =  arrName[j+1]; \
                } \
                arrName[ arrName ## _count -- ] = 0L; \
                break; \
            } \
    }

#define ArrayListRemoveByName(arrName, name) \
    { \
        int i,j; \
        for(i=0; i< arrName ## _count; i++) \
            if(arrName[i] && arrName[i]->x_name == name) { \
                for(j=i; j< arrName ## _count - 1; j++) { \
                    arrName[j] =  arrName[j+1]; \
                } \
                arrName[ arrName ## _count -- ] = 0L; \
                break; \
            } \
    }

#define ArrayListGetByName(arrName, n, type, result) \
    type result = (type) 0L; \
    if(arrName) { \
        int i; \
        for(i=0; i< arrName ## _count; i++) { \
            if(arrName[i] && arrName[i]->x_name == n) { \
                result = arrName[i]; break; \
            } \
        } \
    }

#define ArrayListGetIndexByName(arrName, n, type, result) \
    type result = (type) -1; \
    if(arrName) { \
        int i; \
        for(i=0; i< arrName ## _count; i++) { \
            if(arrName[i] && arrName[i]->x_name == n) { \
                result = i; \
                break; \
            } \
        } \
    }

#define ArrayListFree(arrName, type) \
    freebytes(arrName, arrName ## _maxsize * sizeof(type))

#endif // __ARRAYLIST_H_INCLUDED_
