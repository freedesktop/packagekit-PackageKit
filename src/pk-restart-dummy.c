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

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include <glib/gi18n.h>
#include <pk-common.h>
#include <pk-debug.h>
#include "pk-restart.h"

static void     pk_restart_class_init	(PkRestartClass *klass);
static void     pk_restart_init		(PkRestart      *restart);
static void     pk_restart_finalize	(GObject       *object);

#define PK_RESTART_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), PK_TYPE_RESTART, PkRestartPrivate))

struct PkRestartPrivate
{
	gboolean		dummy;
};

enum {
	PK_RESTART_SCHEDULE,
	PK_RESTART_LAST_SIGNAL
};

static guint	     signals [PK_RESTART_LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (PkRestart, pk_restart, G_TYPE_OBJECT)

/**
 * pk_restart_class_init:
 * @klass: The PkRestartClass
 **/
static void
pk_restart_class_init (PkRestartClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = pk_restart_finalize;

	signals [PK_RESTART_SCHEDULE] =
		g_signal_new ("restart-schedule",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	g_type_class_add_private (klass, sizeof (PkRestartPrivate));
}

/**
 * pk_restart_init:
 * @restart: This class instance
 **/
static void
pk_restart_init (PkRestart *restart)
{
	restart->priv = PK_RESTART_GET_PRIVATE (restart);
}

/**
 * pk_restart_finalize:
 * @object: The object to finalize
 **/
static void
pk_restart_finalize (GObject *object)
{
	PkRestart *restart;

	g_return_if_fail (PK_IS_RESTART (object));

	restart = PK_RESTART (object);
	g_return_if_fail (restart->priv != NULL);

	G_OBJECT_CLASS (pk_restart_parent_class)->finalize (object);
}

/**
 * pk_restart_new:
 *
 * Return value: a new PkRestart object.
 **/
PkRestart *
pk_restart_new (void)
{
	PkRestart *restart;
	restart = g_object_new (PK_TYPE_RESTART, NULL);
	return PK_RESTART (restart);
}

/***************************************************************************
 ***                          MAKE CHECK TESTS                           ***
 ***************************************************************************/
#ifdef PK_BUILD_TESTS
#include <libselftest.h>

void
libst_restart (LibSelfTest *test)
{
	PkRestart *restart;

	if (libst_start (test, "PkRestart", CLASS_AUTO) == FALSE) {
		return;
	}

	/************************************************************/
	libst_title (test, "get a restart");
	restart = pk_restart_new ();
	if (restart != NULL) {
		libst_success (test, NULL);
	} else {
		libst_failed (test, NULL);
	}
	g_object_unref (restart);

	libst_end (test);
}
#endif

