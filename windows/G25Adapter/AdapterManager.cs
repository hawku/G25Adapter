/*
 * Date: 7.8.2015
 * Time: 19:02
 */
using System;
using System.Linq;
using HidLibrary;

namespace G25Adapter
{
	/// <summary>
	/// Description of AdapterManager.
	/// </summary>
	public class AdapterManager
	{
		public static int VendorId = 0x16c0;
		public static int ProductId = 0x0487;
		
		private HidDevice _device;
		private LogListener _logListener;
		private bool _running;
		
		public AdapterManager()
		{
			_running = false;
			_device = null;
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
			_logListener.WriteLog("ADAPTER: " + message);
		}
		
		private HidDevice[] GetDevices()
		{
			return HidDevices.Enumerate(VendorId, ProductId).Cast<HidDevice>().ToArray();
		}
				
		
		public void InitHID()
		{
			HidDevice[] devices;
			devices = GetDevices();
			foreach (HidDevice d in devices) {
				if (d.Capabilities.InputReportByteLength >= 40) {
					WriteLog("Path: " + d.DevicePath);
					WriteLog("Desc: " + d.Description);
					WriteLog("Input Report Size: " + d.Capabilities.InputReportByteLength);
					_device = d;
					break;
				}
			}
			
			_running = true;
		}
		
		public void SendButtons(UInt32 buttons)
		{
			var data = new byte[33];
			if (!_running) {
				return;
			}
			// Report ID
			data[0] = 3;
			
			// Message ID
			data[1] = 1;
			
			// Data
			data[2] = (byte)(buttons & 0xFF);
			data[3] = (byte)((buttons >> 8) & 0xFF);
			data[4] = (byte)((buttons >> 16) & 0xFF);
			data[5] = (byte)((buttons >> 24) & 0xFF);
			
			_device.WriteAsync(data);
			WriteLog("Sent! " + buttons);
		}
			

		public void OnReport(HidReport report)
		{
			if (report.ReportId == 4) {
				WriteLog("Report! " + report.Data[1] + ", " + report.Data[2] + ",  " + report.Data[5] + ", " + report.Data[10]);
			}
			if (_running) {
				_device.ReadReport(OnReport);
			}
		}
	}
}
