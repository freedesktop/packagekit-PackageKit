/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2007-2008 Richard Hughes <richard@hughsie.com>
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

#include <pk-backend.h>
#include <pk-backend-spawn.h>
#include <pk-package-ids.h>

static PkBackendSpawn *spawn;

/**
 * backend_initialize:
 * This should only be run once per backend load, i.e. not every transaction
 */
static void
backend_initialize (PkBackend *backend)
{
	pk_debug ("FILTER: initialize");
	spawn = pk_backend_spawn_new ();
	pk_backend_spawn_set_name (spawn, "urpmi");
}

/**
 * backend_destroy:
 * This should only be run once per backend load, i.e. not every transaction
 */
static void
backend_destroy (PkBackend *backend)
{
	pk_debug ("FILTER: destroy");
	g_object_unref (spawn);
}

/**
 * backend_get_groups:
 */
static PkGroupEnum
backend_get_groups (PkBackend *backend)
{
  return(PK_GROUP_ENUM_UNKNOWN |
  PK_GROUP_ENUM_ACCESSIBILITY |
  PK_GROUP_ENUM_ACCESSORIES |
  PK_GROUP_ENUM_EDUCATION |
  PK_GROUP_ENUM_GAMES |
  PK_GROUP_ENUM_GRAPHICS |
  PK_GROUP_ENUM_INTERNET |
  PK_GROUP_ENUM_OFFICE |
  PK_GROUP_ENUM_OTHER |
  PK_GROUP_ENUM_PROGRAMMING |
  PK_GROUP_ENUM_MULTIMEDIA |
  PK_GROUP_ENUM_SYSTEM |
  PK_GROUP_ENUM_DESKTOP_GNOME |
  PK_GROUP_ENUM_DESKTOP_KDE |
  PK_GROUP_ENUM_DESKTOP_XFCE |
  PK_GROUP_ENUM_DESKTOP_OTHER |
  PK_GROUP_ENUM_PUBLISHING |
  PK_GROUP_ENUM_SERVERS |
  PK_GROUP_ENUM_FONTS |
  PK_GROUP_ENUM_ADMIN_TOOLS |
  PK_GROUP_ENUM_LEGACY |
  PK_GROUP_ENUM_LOCALIZATION |
  PK_GROUP_ENUM_VIRTUALIZATION |
  PK_GROUP_ENUM_POWER_MANAGEMENT |
  PK_GROUP_ENUM_SECURITY |
  PK_GROUP_ENUM_COMMUNICATION |
  PK_GROUP_ENUM_NETWORK |
  PK_GROUP_ENUM_MAPS |
  PK_GROUP_ENUM_REPOS);
}

/**
 * backend_get_filters:
 */
static PkFilterEnum
backend_get_filters (PkBackend *backend)
{
	return (PK_FILTER_ENUM_GUI |
		PK_FILTER_ENUM_INSTALLED |
		PK_FILTER_ENUM_DEVELOPMENT);
}

/**
 * pk_backend_bool_to_text:
 */
static const gchar *
pk_backend_bool_to_text (gboolean value)
{
	if (value == TRUE) {
		return "yes";
	}
	return "no";
}


/**
 * pk_backend_search_name:
 */
static void
backend_search_name (PkBackend *backend, PkFilterEnum filters, const gchar *search)
{
	gchar *filters_text;
	filters_text = pk_filter_enums_to_text (filters);
	pk_backend_spawn_helper (spawn, "search-name.pl", filters_text, search, NULL);
	g_free (filters_text);
}

/**
 * backend_get_details:
 */
static void
backend_get_details (PkBackend *backend, const gchar *package_id)
{
	pk_backend_spawn_helper (spawn, "get-details.pl", package_id, NULL);
}

/**
 * backend_get_files:
 */
static void
backend_get_files (PkBackend *backend, const gchar *package_id)
{
	pk_backend_spawn_helper (spawn, "get-files.pl", package_id, NULL);
}

/**
 * backend_get_depends:
 */
static void
backend_get_depends (PkBackend *backend, PkFilterEnum filters, const gchar *package_id, gboolean recursive)
{
	gchar *filters_text;
	filters_text = pk_filter_enums_to_text (filters);
	pk_backend_spawn_helper (spawn, "get-depends.pl", filters_text, package_id, pk_backend_bool_to_text (recursive), NULL);
	g_free (filters_text);
}

/**
 * backend_get_updates:
 */
static void
backend_get_updates (PkBackend *backend, PkFilterEnum filters)
{
	gchar *filters_text;
	filters_text = pk_filter_enums_to_text (filters);
	pk_backend_spawn_helper (spawn, "get-updates.pl", filters_text, NULL);
	g_free (filters_text);
}

/**
 * backend_get_update_detail:
 */
static void
backend_get_update_detail (PkBackend *backend, const gchar *package_id)
{
	pk_backend_spawn_helper (spawn, "get-update-detail.pl", package_id, NULL);
}

/**
 * backend_refresh_cache:
 */
static void
backend_refresh_cache (PkBackend *backend, gboolean force)
{
	/* check network state */
	if (!pk_backend_is_online (backend)) {
		pk_backend_error_code (backend, PK_ERROR_ENUM_NO_NETWORK, "Cannot refresh cache whilst offline");
		pk_backend_finished (backend);
		return;
	}

	pk_backend_spawn_helper (spawn, "refresh-cache.pl", NULL);
}

/**
 * backend_install_packages:
 */
static void
backend_install_packages (PkBackend *backend, gchar **package_ids)
{
	gchar *package_ids_temp;

	/* check network state */
	if (!pk_backend_is_online (backend)) {
		pk_backend_error_code (backend, PK_ERROR_ENUM_NO_NETWORK, "Cannot install when offline");
		pk_backend_finished (backend);
		return;
	}

	/* send the complete list as stdin */
	package_ids_temp = pk_package_ids_to_text (package_ids, "|");
	pk_backend_spawn_helper (spawn, "install-packages.pl", package_ids_temp, NULL);
	g_free (package_ids_temp);
}

/**
 * pk_backend_remove_packages:
 */
static void
backend_remove_packages (PkBackend *backend, gchar **package_ids, gboolean allow_deps, gboolean autoremove)
{
	gchar *package_ids_temp;

	/* send the complete list as stdin */
	package_ids_temp = pk_package_ids_to_text (package_ids, "|");
	pk_backend_spawn_helper (spawn, "remove-packages.pl", pk_backend_bool_to_text (allow_deps), package_ids_temp, NULL);
	g_free (package_ids_temp);
}

/**
 * pk_backend_search_group:
 */
static void
backend_search_group (PkBackend *backend, PkFilterEnum filters, const gchar *search)
{
	gchar *filters_text;
	filters_text = pk_filter_enums_to_text (filters);
	pk_backend_spawn_helper (spawn, "search-group.pl", filters_text, search, NULL);
	g_free (filters_text);
}

/**
 * backend_get_packages:
 */
static void
backend_get_packages (PkBackend *backend, PkFilterEnum filters)
{
	gchar *filters_text;
	filters_text = pk_filter_enums_to_text (filters);
	pk_backend_spawn_helper (spawn, "get-packages.pl", filters_text, NULL);
	g_free (filters_text);
}

/**
 * backend_get_requires:
 */
static void
backend_get_requires (PkBackend *backend, PkFilterEnum filters, const gchar *package_id, gboolean recursive)
{
	gchar *filters_text;
	filters_text = pk_filter_enums_to_text (filters);
	pk_backend_spawn_helper (spawn, "get-requires.pl", filters_text, package_id, pk_backend_bool_to_text (recursive), NULL);
	g_free (filters_text);
}

/**
 * pk_backend_search_details:
 */
static void
backend_search_details (PkBackend *backend, PkFilterEnum filters, const gchar *search)
{
	gchar *filters_text;
	filters_text = pk_filter_enums_to_text (filters);
	pk_backend_spawn_helper (spawn, "search-details.pl", filters_text, search, NULL);
	g_free (filters_text);
}

/**
 * pk_backend_search_file:
 */
static void
backend_search_file (PkBackend *backend, PkFilterEnum filters, const gchar *search)
{
	gchar *filters_text;
	filters_text = pk_filter_enums_to_text (filters);
	pk_backend_spawn_helper (spawn, "search-file.pl", filters_text, search, NULL);
	g_free (filters_text);
}

/**
 * pk_backend_resolve:
 */
static void
backend_resolve (PkBackend *backend, PkFilterEnum filters, const gchar *package_id)
{
	gchar *filters_text;
	filters_text = pk_filter_enums_to_text (filters);
	pk_backend_spawn_helper (spawn, "resolve.pl", filters_text, package_id, NULL);
	g_free (filters_text);
}

/**
 * pk_backend_update_packages:
 */
static void
backend_update_packages (PkBackend *backend, gchar **package_ids)
{
	gchar *package_ids_temp;


	/* check network state */
	if (!pk_backend_is_online (backend)) {
		pk_backend_error_code (backend, PK_ERROR_ENUM_NO_NETWORK, "Cannot install when offline");
		pk_backend_finished (backend);
		return;
	}

	/* send the complete list as stdin */
	package_ids_temp = pk_package_ids_to_text (package_ids, "|");
	pk_backend_spawn_helper (spawn, "update-packages.pl", package_ids_temp, NULL);
	g_free (package_ids_temp);
}

/**
 * pk_backend_update_system:
 */
static void
backend_update_system (PkBackend *backend)
{
	pk_backend_spawn_helper (spawn, "update-system.pl", NULL);
}


PK_BACKEND_OPTIONS (
	"URPMI",					/* description */
	"Aurelien Lefebvre <alefebvre@mandriva.com>",	/* author */
	backend_initialize,			/* initalize */
	backend_destroy,			/* destroy */
	backend_get_groups,			/* get_groups */
	backend_get_filters,			/* get_filters */
	NULL,				/* cancel */
	backend_get_depends,			/* get_depends */
	backend_get_details,			/* get_details */
	backend_get_files,			/* get_files */
	backend_get_packages,			/* get_packages */
	NULL,			/* get_repo_list */
	backend_get_requires,			/* get_requires */
	backend_get_update_detail,		/* get_update_detail */
	backend_get_updates,			/* get_updates */
	NULL,			/* install_files */
	backend_install_packages,		/* install_packages */
	NULL,		/* install_signature */
	backend_refresh_cache,			/* refresh_cache */
	backend_remove_packages,		/* remove_packages */
	NULL,			/* repo_enable */
	NULL,			/* repo_set_data */
	backend_resolve,			/* resolve */
	NULL,					/* rollback */
	backend_search_details,			/* search_details */
	backend_search_file,			/* search_file */
	backend_search_group,			/* search_group */
	backend_search_name,			/* search_name */
	NULL,					/* service_pack */
	backend_update_packages,		/* update_packages */
	backend_update_system,			/* update_system */
	NULL			/* what_provides */
);
