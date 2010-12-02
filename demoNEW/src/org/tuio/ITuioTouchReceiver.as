package org.tuio
{

	public interface ITuioTouchReceiver
	{
		function updateTouch(event:TouchEvent):void;
		function removeTouch(event:TouchEvent):void;
	}
}