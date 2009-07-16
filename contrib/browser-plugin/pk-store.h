/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2009 Richard Hughes <richard@hughsie.com>
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

#ifndef __PK_STORE_H
#define __PK_STORE_H

#include <glib-object.h>
#include <npapi.h>

#include "pk-plugin.h"

G_BEGIN_DECLS

#define PK_TYPE_STORE		(pk_store_get_type ())
#define PK_STORE(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), PK_TYPE_STORE, PkStore))
#define PK_STORE_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), PK_TYPE_STORE, PkStoreClass))
#define PK_IS_STORE(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), PK_TYPE_STORE))
#define PK_IS_STORE_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), PK_TYPE_STORE))
#define PK_STORE_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), PK_TYPE_STORE, PkStoreClass))

typedef struct PkStorePrivate PkStorePrivate;

typedef struct
{
	GObject		 parent;
	PkStorePrivate	*priv;
} PkStore;

typedef struct
{
	GObjectClass	 parent_class;
} PkStoreClass;

GType		 pk_store_get_type		(void);
PkStore		*pk_store_new			(void);
gboolean	 pk_store_add_plugin		(PkStore	*store,
						 NPP		 instance,
						 PkPlugin	*plugin);
gboolean	 pk_store_remove_plugin		(PkStore	*store,
						 NPP		 instance);
PkPlugin	*pk_store_lookup_plugin		(PkStore	*store,
						 NPP		 instance);

G_END_DECLS

#endif /* __PK_STORE_H */