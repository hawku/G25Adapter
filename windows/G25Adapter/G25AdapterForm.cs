/*
 * Date: 7.8.2015
 * Time: 19:00
 */
using System;
using System.Drawing;
using System.Windows.Forms;
using SharpDX.DirectInput;

namespace G25Adapter
{
	/// <summary>
	/// Description of G25AdapterForm.
	/// </summary>
	public partial class G25AdapterForm : Form, JoystickListener, LogListener
	{
		private uint buttons, oldButtons;
		private AdapterManager adapterManager;
		private JoystickManager[] joystickManagers;
		
		public G25AdapterForm()
		{
			InitializeComponent();
			buttons = 0;
			oldButtons = 0;
			adapterManager = new AdapterManager();
			adapterManager.SetLogListener(this);
			FetchJoystickList();
		}
		
		public void FetchJoystickList()
		{
			listBoxJoysticks.Items.Clear();
			foreach (var joystick in JoystickManager.GetJoysticks()) {
				listBoxJoysticks.Items.Add(joystick.InstanceName);
			}
			listBoxJoysticks.SelectedIndex = 0;

		}

		void ButtonStartClick(object sender, EventArgs e)
		{
			adapterManager.InitHID();
			
			joystickManagers = new JoystickManager[listBoxJoysticks.SelectedIndices.Count];
			for(int i = 0; i < listBoxJoysticks.SelectedIndices.Count; i++) {
				joystickManagers[i] = new JoystickManager();
				joystickManagers[i].SetJoystickByName(listBoxJoysticks.SelectedItems[i].ToString());
 				joystickManagers[i].SetListener(this);
				joystickManagers[i].StartPolling();
			}
			buttonStart.Enabled = false;
			buttonStop.Enabled = true;
	
		}
		void ButtonStopClick(object sender, EventArgs e)
		{
			if (joystickManagers != null && joystickManagers[0] != null) {
				foreach(JoystickManager joystickManager in joystickManagers) {
					joystickManager.StopPolling();			
				}
			}
			buttonStart.Enabled = true;
			buttonStop.Enabled = false;
			
		}
		
		public void WriteLog(string message)
		{
			if (this.InvokeRequired) {
				Invoke(new MethodInvoker(delegate() {
					WriteLog(message);
				}));
			} else {
				listBoxLog.Items.Add(message);
				listBoxLog.SelectedIndex = this.listBoxLog.Items.Count - 1;
			}
		}
		
		public void OnJoystickUpdate(JoystickUpdate[] updates)
		{
			int button;
			bool down;
			foreach (JoystickUpdate update in updates) {

				// Buttons 1 & 2				
				if (update.Offset >= JoystickOffset.Buttons0 && update.Offset <= JoystickOffset.Buttons1) {
					button = update.Offset - JoystickOffset.Buttons0 + 12;
					down = update.Value > 0;
					if (down) {
						buttons |= ((uint)1 << button);
					} else {
						buttons &= ~((uint)1 << button);
					}
					Console.WriteLine("Button: " + button + ", Down = " + down);
				}
			}
			if (buttons != oldButtons) {
				adapterManager.SendButtons(buttons);
				oldButtons = buttons;
			}
		}
		
	}
}
