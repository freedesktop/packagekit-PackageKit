// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace PackageKit {

	using System;

	public delegate void StatusChangedHandler(object o, StatusChangedArgs args);

	public class StatusChangedArgs : GLib.SignalArgs {
		public PackageKit.StatusEnum Status{
			get {
				return (PackageKit.StatusEnum) Args[0];
			}
		}

	}
}