/*
 * Date: 7.8.2015
 * Time: 18:15
 */
using System;
using SharpDX.DirectInput;

namespace G25Adapter
{
	/// <summary>
	/// Description of JoystickListener.
	/// </summary>
	public interface JoystickListener
	{
		void OnJoystickUpdate(JoystickUpdate[] updates);
	}
}
