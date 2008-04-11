/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2008 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __PK_CACHE_H
#define __PK_CACHE_H

#include <glib-object.h>
#include <pk-package-list.h>

G_BEGIN_DECLS

#define PK_TYPE_CACHE		(pk_cache_get_type ())
#define PK_CACHE(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), PK_TYPE_CACHE, PkCache))
#define PK_CACHE_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), PK_TYPE_CACHE, PkCacheClass))
#define PK_IS_CACHE(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), PK_TYPE_CACHE))
#define PK_IS_CACHE_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), PK_TYPE_CACHE))
#define PK_CACHE_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), PK_TYPE_CACHE, PkCacheClass))

typedef struct PkCachePrivate PkCachePrivate;

typedef struct
{
	GObject		      parent;
	PkCachePrivate	     *priv;
} PkCache;

typedef struct
{
	GObjectClass	parent_class;
} PkCacheClass;

GType		 pk_cache_get_type		(void) G_GNUC_CONST;
PkCache		*pk_cache_new			(void);

PkPackageList	*pk_cache_get_updates		(PkCache	*cache);
gboolean	 pk_cache_set_updates		(PkCache	*cache,
						 PkPackageList	*list);
gboolean	 pk_cache_invalidate		(PkCache	*cache);

G_END_DECLS

#endif /* __PK_CACHE_H */