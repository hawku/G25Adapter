/*
 * Date: 7.8.2015
 * Time: 18:15
 */
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Threading;
using SharpDX.DirectInput;

namespace G25Adapter
{
	/// <summary>
	/// Description of JoystickManager.
	/// </summary>
	public class JoystickManager
	{
		public static DirectInput directInput = new DirectInput();
		public Joystick joystick;
		public Guid joystickGuid;
		
		private JoystickListener _listener;
		private bool _running;
		private LogListener _logListener;
		private BackgroundWorker _worker;
		
		
		public JoystickManager()
		{
			joystick = null;
			joystickGuid = Guid.Empty;
			_listener = null;
			_running = false;
			_logListener = null;
			_worker = new BackgroundWorker();
			_worker.WorkerReportsProgress = false;
			_worker.WorkerSupportsCancellation = true;
			_worker.DoWork += new DoWorkEventHandler(delegate(object o, DoWorkEventArgs args) {
				while (_running) {
					DoPoll();
					Thread.Sleep(1);
				}
			});
			
		}
						
		public void SetLogListener(LogListener listener)
		{
			_logListener = listener;
		}
		
		private void WriteLog(string message)
		{
			if (_logListener == null) {
				return;
			}
			_logListener.WriteLog("JOYSTICK: " + message);
		}
		
		public void SetListener(JoystickListener listener)
		{
			this._listener = listener;
		}
		public static IList<DeviceInstance> GetJoysticks()
		{
			return directInput.GetDevices(DeviceClass.GameControl, DeviceEnumerationFlags.AttachedOnly);
		}
		
		public bool SetJoystickByName(string name)
		{
			
			WriteLog("Searching joystick!");
			
			// Find joystick
			foreach (DeviceInstance deviceInstance in GetJoysticks()) {
				if (deviceInstance.InstanceName.StartsWith(name, StringComparison.CurrentCulture)) {
					joystickGuid = deviceInstance.InstanceGuid;
				}
			}
			
			// Joystick not found!
			if (joystickGuid == Guid.Empty) {
				WriteLog("Joystick not found!");
				return false;
			}
			WriteLog("Joystick found!");
		
			// Create & acquire the joystick			
			joystick = new Joystick(directInput, joystickGuid);
			joystick.Properties.BufferSize = 128;
			joystick.Acquire();
			
			WriteLog("Joystick '" + joystick.Properties.InstanceName + "' found!");
			
			return true;
		}
		
		private void DoPoll()
		{
			joystick.Poll();
			JoystickUpdate[] updates = joystick.GetBufferedData();
			if (updates != null && updates.Length > 0) {
				_listener.OnJoystickUpdate(updates);
			}
		}
		
		public void StartPolling()
		{
			_running = true;
			_worker.RunWorkerAsync();
		}
		public void StopPolling()
		{
			_running = false;
			_worker.CancelAsync();
		}
	}
}
