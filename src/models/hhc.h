/*
 *  Copyright (c) 2006           Ji YongGang <jungle@soforge-studio.com>
 *
 *  ChmSee is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.

 *  ChmSee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with ChmSee; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */


#ifndef __CHMSEE_MODELS_HHC_H__
#define __CHMSEE_MODELS_HHC_H__

#include <glib.h>

typedef GNode Hhc;

/** 
 * load Hhc from file
 * 
 * @param filename 
 * @param encoding 
 * 
 * @return 
 */
Hhc* hhc_load(const gchar* filename, const gchar* encoding);

/** 
 * free Hhc
 * 
 * @param self 
 */
void hhc_free(Hhc* self);

#endif /* !__PARSER_H__ */
