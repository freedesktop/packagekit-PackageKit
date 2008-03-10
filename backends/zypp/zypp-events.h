#ifndef _ZYPP_EVENTS_H_
#define _ZYPP_EVENTS_H_

#include <stdio.h>
#include <glib.h>
#include <pk-backend.h>
#include <zypp/ZYppCallbacks.h>

#include "zypp-utils.h"

/*
typedef struct {
	PkBackend *backend;
	guint percentage;
} PercentageData;

static gboolean
emit_sub_percentage (gpointer data)
{
	PercentageData *pd = (PercentageData *)data;
	pk_backend_set_sub_percentage (pd->backend, pd->percentage);
	free (pd);
	return FALSE;
}
*/

namespace ZyppBackend
{

struct ZyppBackendReceiver
{
	PkBackend *_backend;
	gchar *_package_id;
	guint _sub_percentage;

	virtual void initWithBackend (PkBackend *backend)
	{
		_backend = backend;
		_package_id = NULL;
		_sub_percentage = 0;
	}

	virtual void clear_package_id ()
	{
		if (_package_id != NULL) {
			g_free (_package_id);
			_package_id = NULL;
		}
	}

	/**
	 * Build a package_id from the specified zypp::Url.  The returned
	 * gchar * should be freed with g_free ().  Returns NULL if the
	 * URL does not contain information about an RPM.
	 *
	 * Example:
	 *    basename: lynx-2.8.6-63.i586.rpm
	 *    result:   lynx;2.8.6-63;i586;opensuse
	 */
	gchar *
	build_package_id_from_url (const zypp::Url *url)
	{
		gchar *package_id;
		gchar *basename;
		gchar *tmp;
	
		gchar *arch;
		gchar *edition;
		gchar *name;
		gboolean first_dash_found;
	
		basename = g_strdup (zypp::Pathname (url->getPathName ()).basename ().c_str());
	
		tmp = g_strrstr (basename, ".rpm");
	
		if (tmp == NULL) {
			g_free (basename);
			return NULL;
		}
	
		// Parse the architecture
		tmp [0] = '\0'; // null-terminate the arch section
		for (tmp--; tmp != basename && tmp [0] != '.'; tmp--) {}
		arch = tmp + 1;
	
		// Parse the edition
		tmp [0] = '\0'; // null-terminate the edition (version)
		first_dash_found = FALSE;
		for (tmp--; tmp != basename; tmp--) {
			if (tmp [0] == '-') {
				if (first_dash_found == FALSE) {
					first_dash_found = TRUE;
					continue;
				} else {
					break;
				}
			}
		}
		edition = tmp + 1;
	
		// Parse the name
		tmp [0] = '\0'; // null-terminate the name
		name = basename;
	
		package_id = pk_package_id_build (name, edition, arch, "opensuse");
		g_free (basename);
	
		return package_id;
	}

	inline void
	update_sub_percentage (guint percentage)
	{
		// TODO: Figure out this weird bug that libzypp emits a 100
		// at the beginning of installing a package.
		if (_sub_percentage == 0 && percentage == 100)
			return; // can't jump from 0 -> 100 instantly!

		// Only emit a percentage if it's different from the last
		// percentage we emitted and it's divisible by ten.  We
		// don't want to overload dbus/GUI.  Also account for the
		// fact that libzypp may skip over a "divisible by ten"
		// value (i.e., 28, 29, 31, 32).

		// Drop off the least significant digit
		// TODO: Figure out a faster way to drop the least significant digit
		percentage = (percentage / 10) * 10;

		if (percentage <= _sub_percentage)
			return;

		_sub_percentage = percentage;
		//PercentageData *pd = (PercentageData *)malloc (sizeof (PercentageData));
		//pd->backend = _backend;
		//pd->percentage = _sub_percentage;
		//g_idle_add (emit_sub_percentage, pd);
		pk_backend_set_sub_percentage (_backend, _sub_percentage);
	}

	void
	reset_sub_percentage ()
	{
		_sub_percentage = 0;
		pk_backend_set_sub_percentage (_backend, _sub_percentage);
	}
};

struct InstallResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::rpm::InstallResolvableReport>, ZyppBackendReceiver
{
	zypp::Resolvable::constPtr _resolvable;

	virtual void start (zypp::Resolvable::constPtr resolvable)
	{
		clear_package_id ();
		_package_id = zypp_build_package_id_from_resolvable (resolvable->satSolvable ());
		//fprintf (stderr, "\n\n----> InstallResolvableReportReceiver::start(): %s\n\n", _package_id == NULL ? "unknown" : _package_id);
		if (_package_id != NULL) {
			pk_backend_set_status (_backend, PK_STATUS_ENUM_INSTALL);
			pk_backend_package (_backend, PK_INFO_ENUM_INSTALLING, _package_id, "TODO: Put the package summary here if possible");
			reset_sub_percentage ();
		}
	}

	virtual bool progress (int value, zypp::Resolvable::constPtr resolvable)
	{
		//fprintf (stderr, "\n\n----> InstallResolvableReportReceiver::progress(), %s:%d\n\n", _package_id == NULL ? "unknown" : _package_id, value);
		if (_package_id != NULL)
			update_sub_percentage (value);
		return true;
	}

	virtual Action problem (zypp::Resolvable::constPtr resolvable, Error error, const std::string &description, RpmLevel level)
	{
		//fprintf (stderr, "\n\n----> InstallResolvableReportReceiver::problem()\n\n");
		return ABORT;
	}

	virtual void finish (zypp::Resolvable::constPtr resolvable, Error error, const std::string &reason, RpmLevel level)
	{
		//fprintf (stderr, "\n\n----> InstallResolvableReportReceiver::finish(): %s\n\n", _package_id == NULL ? "unknown" : _package_id);
		if (_package_id != NULL) {
			pk_backend_package (_backend, PK_INFO_ENUM_INSTALLED, _package_id, "TODO: Put the package summary here if possible");
			clear_package_id ();
		}
	}
};

struct RemoveResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::rpm::RemoveResolvableReport>, ZyppBackendReceiver
{
	zypp::Resolvable::constPtr _resolvable;

	virtual void start (zypp::Resolvable::constPtr resolvable)
	{
		clear_package_id ();
		_package_id = zypp_build_package_id_from_resolvable (resolvable->satSolvable ());
		if (_package_id != NULL) {
			pk_backend_set_status (_backend, PK_STATUS_ENUM_REMOVE);
			pk_backend_package (_backend, PK_INFO_ENUM_REMOVING, _package_id, "");
			reset_sub_percentage ();
		}
	}

	virtual bool progress (int value, zypp::Resolvable::constPtr resolvable)
	{
		if (_package_id != NULL)
			update_sub_percentage (value);
		return true;
	}

	virtual Action problem (zypp::Resolvable::constPtr resolvable, Error error, const std::string &description)
	{
                pk_backend_error_code (_backend, PK_ERROR_ENUM_CANNOT_REMOVE_SYSTEM_PACKAGE, description.c_str ());
		return ABORT;
	}

	virtual void finish (zypp::Resolvable::constPtr resolvable, Error error, const std::string &reason)
	{
		if (_package_id != NULL) {
			pk_backend_package (_backend, PK_INFO_ENUM_AVAILABLE, _package_id, "");
			clear_package_id ();
		}
	}
};

struct RepoProgressReportReceiver : public zypp::callback::ReceiveReport<zypp::ProgressReport>, ZyppBackendReceiver
{
	virtual void start (const zypp::ProgressData &data)
	{
		pk_debug ("_____________- RepoProgressReportReceiver::start()___________________");
		reset_sub_percentage ();
	}

	virtual bool progress (const zypp::ProgressData &data)
	{
		//fprintf (stderr, "\n\n----> RepoProgressReportReceiver::progress(), %s:%d\n\n", data.name().c_str(), (int)data.val());
		update_sub_percentage ((int)data.val ());
		return true;
	}

	virtual void finish (const zypp::ProgressData &data)
	{
		//fprintf (stderr, "\n\n----> RepoProgressReportReceiver::finish()\n\n");
	}
};

struct RepoReportReceiver : public zypp::callback::ReceiveReport<zypp::repo::RepoReport>, ZyppBackendReceiver
{
	virtual void start (const zypp::ProgressData &data)
	{
		pk_debug ("______________________ RepoReportReceiver::start()________________________");
		reset_sub_percentage ();
	}

	virtual bool progress (const zypp::ProgressData &data)
	{
		//fprintf (stderr, "\n\n----> RepoReportReceiver::progress(), %s:%d\n", data.name().c_str(), (int)data.val());
		update_sub_percentage ((int)data.val ());
		return true;
	}

	virtual void finish (const zypp::ProgressData &data)
	{
		//fprintf (stderr, "\n\n----> RepoReportReceiver::finish()\n");
	}
};

struct DownloadProgressReportReceiver : public zypp::callback::ReceiveReport<zypp::media::DownloadProgressReport>, ZyppBackendReceiver
{
	virtual void start (const zypp::Url &file, zypp::Pathname localfile)
	{
		clear_package_id ();
		_package_id = build_package_id_from_url (&file);

		//fprintf (stderr, "\n\n----> DownloadProgressReportReceiver::start(): %s\n", _package_id == NULL ? "unknown" : _package_id);
		if (_package_id != NULL) {
			pk_backend_set_status (_backend, PK_STATUS_ENUM_DOWNLOAD);
			pk_backend_package (_backend, PK_INFO_ENUM_DOWNLOADING, _package_id, "TODO: Put the package summary here if possible");
			reset_sub_percentage ();
		}
	}

	virtual bool progress (int value, const zypp::Url &file)
	{
		//fprintf (stderr, "\n\n----> DownloadProgressReportReceiver::progress(), %s:%d\n\n", _package_id == NULL ? "unknown" : _package_id, value);
		if (_package_id != NULL)
			update_sub_percentage (value);
		return true;
	}

	virtual void finish (const zypp::Url & file, Error error, const std::string &konreason)
	{
		//fprintf (stderr, "\n\n----> DownloadProgressReportReceiver::finish(): %s\n", _package_id == NULL ? "unknown" : _package_id);
		clear_package_id ();
	}
};

struct KeyRingReportReceiver : public zypp::callback::ReceiveReport<zypp::KeyRingReport>, ZyppBackendReceiver
{
        virtual bool askUserToAcceptUnsignedFile (const std::string &file)
        {
                gboolean ok = zypp_signature_required(_backend, file);

                return ok;
        }

        virtual bool askUserToAcceptUnknownKey (const std::string &file, const std::string &id)
        {
                gboolean ok = zypp_signature_required(_backend, file, id);

                return ok;
        }

        virtual bool askUserToTrustKey (const zypp::PublicKey &key)
        {
                gboolean ok = zypp_signature_required(_backend, key);

                return ok;
        }

        virtual bool askUserToImportKey (const zypp::PublicKey &key)
        {
                gboolean ok = zypp_signature_required(_backend, key);

                return ok;
        }

};

struct MediaChangeReportReceiver : public zypp::callback::ReceiveReport<zypp::media::MediaChangeReport>, ZyppBackendReceiver
{
        virtual Action requestMedia (zypp::Url &url, unsigned mediaNr, zypp::media::MediaChangeReport::Error error, const std::string &probl)
        {
                pk_backend_error_code (_backend,
                                PK_ERROR_ENUM_PACKAGE_DOWNLOAD_FAILED,
                                probl.c_str ());
                // We've to abort here, because there is currently no feasible way to inform the user to insert/change media
                return ABORT;
        }
};

struct ProgressReportReceiver : public zypp::callback::ReceiveReport<zypp::ProgressReport>, ZyppBackendReceiver
{
        virtual void start (const zypp::ProgressData &progress)
        {
		pk_debug ("__________________________ProgressReportReceiver____________________________");
                reset_sub_percentage ();
        }

        virtual bool progress (const zypp::ProgressData &progress)
        {
                update_sub_percentage ((int)progress.val ());
		return true;
        }

        virtual void finish (const zypp::ProgressData &progress)
        {
                update_sub_percentage ((int)progress.val ());
        }
};

}; // namespace ZyppBackend

class EventDirector
{
	private:
		EventDirector () {}
	
		ZyppBackend::RepoReportReceiver _repoReport;
		ZyppBackend::RepoProgressReportReceiver _repoProgressReport;
		ZyppBackend::InstallResolvableReportReceiver _installResolvableReport;
		ZyppBackend::RemoveResolvableReportReceiver _removeResolvableReport;
		ZyppBackend::DownloadProgressReportReceiver _downloadProgressReport;
                ZyppBackend::KeyRingReportReceiver _keyRingReport;
                ZyppBackend::MediaChangeReportReceiver _mediaChangeReport;
                ZyppBackend::ProgressReportReceiver _progressReport;

	public:
		EventDirector (PkBackend *backend)
		{
			_repoReport.initWithBackend (backend);
			_repoReport.connect ();

			_repoProgressReport.initWithBackend (backend);
			_repoProgressReport.connect ();

			_installResolvableReport.initWithBackend (backend);
			_installResolvableReport.connect ();

			_removeResolvableReport.initWithBackend (backend);
			_removeResolvableReport.connect ();
                        
                        _downloadProgressReport.initWithBackend (backend);
			_downloadProgressReport.connect ();

                        _keyRingReport.initWithBackend (backend);
                        _keyRingReport.connect ();

                        _mediaChangeReport.initWithBackend (backend);
                        _mediaChangeReport.connect ();
                        
                        _progressReport.initWithBackend (backend);
                        _progressReport.connect ();
		}

		~EventDirector ()
		{
			_repoReport.disconnect ();
			_repoProgressReport.disconnect ();
			_installResolvableReport.disconnect ();
			_removeResolvableReport.disconnect ();
			_downloadProgressReport.disconnect ();
                        _keyRingReport.disconnect ();
                        _mediaChangeReport.disconnect ();
                        _progressReport.disconnect ();
		}
};


#endif // _ZYPP_EVENTS_H_

