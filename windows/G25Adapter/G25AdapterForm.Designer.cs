/*
 * Date: 7.8.2015
 * Time: 19:00
 */
namespace G25Adapter
{
	partial class G25AdapterForm
	{
		/// <summary>
		/// Designer variable used to keep track of non-visual components.
		/// </summary>
		private System.ComponentModel.IContainer components = null;
		private System.Windows.Forms.StatusStrip statusStrip;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel;
		private System.Windows.Forms.ListBox listBoxLog;
		private System.Windows.Forms.GroupBox groupBoxLog;
		private System.Windows.Forms.Button buttonStart;
		private System.Windows.Forms.Button buttonStop;
		private System.Windows.Forms.GroupBox groupBoxJoystick;
		private System.Windows.Forms.ListBox listBoxJoysticks;
		
		/// <summary>
		/// Disposes resources used by the form.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing) {
				if (components != null) {
					components.Dispose();
				}
			}
			base.Dispose(disposing);
		}
		
		/// <summary>
		/// This method is required for Windows Forms designer support.
		/// Do not change the method contents inside the source code editor. The Forms designer might
		/// not be able to load this method if it was changed manually.
		/// </summary>
		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(G25AdapterForm));
			this.statusStrip = new System.Windows.Forms.StatusStrip();
			this.toolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
			this.listBoxLog = new System.Windows.Forms.ListBox();
			this.groupBoxLog = new System.Windows.Forms.GroupBox();
			this.buttonStart = new System.Windows.Forms.Button();
			this.buttonStop = new System.Windows.Forms.Button();
			this.groupBoxJoystick = new System.Windows.Forms.GroupBox();
			this.listBoxJoysticks = new System.Windows.Forms.ListBox();
			this.statusStrip.SuspendLayout();
			this.groupBoxLog.SuspendLayout();
			this.groupBoxJoystick.SuspendLayout();
			this.SuspendLayout();
			// 
			// statusStrip
			// 
			this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
			this.toolStripStatusLabel});
			this.statusStrip.Location = new System.Drawing.Point(0, 445);
			this.statusStrip.Name = "statusStrip";
			this.statusStrip.Size = new System.Drawing.Size(587, 22);
			this.statusStrip.TabIndex = 0;
			this.statusStrip.Text = "statusStrip1";
			// 
			// toolStripStatusLabel
			// 
			this.toolStripStatusLabel.Name = "toolStripStatusLabel";
			this.toolStripStatusLabel.Size = new System.Drawing.Size(0, 17);
			// 
			// listBoxLog
			// 
			this.listBoxLog.Dock = System.Windows.Forms.DockStyle.Fill;
			this.listBoxLog.FormattingEnabled = true;
			this.listBoxLog.Location = new System.Drawing.Point(3, 16);
			this.listBoxLog.Name = "listBoxLog";
			this.listBoxLog.Size = new System.Drawing.Size(557, 229);
			this.listBoxLog.TabIndex = 1;
			// 
			// groupBoxLog
			// 
			this.groupBoxLog.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
			| System.Windows.Forms.AnchorStyles.Left) 
			| System.Windows.Forms.AnchorStyles.Right)));
			this.groupBoxLog.Controls.Add(this.listBoxLog);
			this.groupBoxLog.Location = new System.Drawing.Point(12, 194);
			this.groupBoxLog.Name = "groupBoxLog";
			this.groupBoxLog.Size = new System.Drawing.Size(563, 248);
			this.groupBoxLog.TabIndex = 2;
			this.groupBoxLog.TabStop = false;
			this.groupBoxLog.Text = "Log";
			// 
			// buttonStart
			// 
			this.buttonStart.Location = new System.Drawing.Point(6, 19);
			this.buttonStart.Name = "buttonStart";
			this.buttonStart.Size = new System.Drawing.Size(75, 38);
			this.buttonStart.TabIndex = 3;
			this.buttonStart.Text = "Start";
			this.buttonStart.UseVisualStyleBackColor = true;
			this.buttonStart.Click += new System.EventHandler(this.ButtonStartClick);
			// 
			// buttonStop
			// 
			this.buttonStop.Enabled = false;
			this.buttonStop.Location = new System.Drawing.Point(6, 63);
			this.buttonStop.Name = "buttonStop";
			this.buttonStop.Size = new System.Drawing.Size(75, 38);
			this.buttonStop.TabIndex = 4;
			this.buttonStop.Text = "Stop";
			this.buttonStop.UseVisualStyleBackColor = true;
			this.buttonStop.Click += new System.EventHandler(this.ButtonStopClick);
			// 
			// groupBoxJoystick
			// 
			this.groupBoxJoystick.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
			| System.Windows.Forms.AnchorStyles.Right)));
			this.groupBoxJoystick.Controls.Add(this.listBoxJoysticks);
			this.groupBoxJoystick.Controls.Add(this.buttonStart);
			this.groupBoxJoystick.Controls.Add(this.buttonStop);
			this.groupBoxJoystick.Location = new System.Drawing.Point(12, 12);
			this.groupBoxJoystick.Name = "groupBoxJoystick";
			this.groupBoxJoystick.Size = new System.Drawing.Size(563, 150);
			this.groupBoxJoystick.TabIndex = 5;
			this.groupBoxJoystick.TabStop = false;
			this.groupBoxJoystick.Text = "Joystick";
			// 
			// listBoxJoysticks
			// 
			this.listBoxJoysticks.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
			| System.Windows.Forms.AnchorStyles.Left) 
			| System.Windows.Forms.AnchorStyles.Right)));
			this.listBoxJoysticks.FormattingEnabled = true;
			this.listBoxJoysticks.Location = new System.Drawing.Point(87, 19);
			this.listBoxJoysticks.Name = "listBoxJoysticks";
			this.listBoxJoysticks.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
			this.listBoxJoysticks.Size = new System.Drawing.Size(470, 121);
			this.listBoxJoysticks.TabIndex = 5;
			// 
			// G25AdapterForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(587, 467);
			this.Controls.Add(this.groupBoxJoystick);
			this.Controls.Add(this.groupBoxLog);
			this.Controls.Add(this.statusStrip);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "G25AdapterForm";
			this.Text = "G25 Adapter Manager";
			this.statusStrip.ResumeLayout(false);
			this.statusStrip.PerformLayout();
			this.groupBoxLog.ResumeLayout(false);
			this.groupBoxJoystick.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}
	}
}
