package tuio.gestures {
	
	import flash.display.DisplayObject;
	import flash.utils.getTimer;
	import tuio.TuioContainer;
	import tuio.TuioEvent;
	import tuio.TouchEvent;
	
	public class ThreeFingerMoveGesture extends Gesture {
		
		public function ThreeFingerMoveGesture() {
			this.addStep(new GestureStep(TouchEvent.TOUCH_MOVE, {tuioContainerAlias:"A", frameIDAlias:"!A"}));
			this.addStep(new GestureStep(TouchEvent.TOUCH_MOVE, { tuioContainerAlias:"B", frameIDAlias:"A"} ));
			this.addStep(new GestureStep(TouchEvent.TOUCH_MOVE, { tuioContainerAlias:"C", frameIDAlias:"A"} ));
			this.addStep(new GestureStep(TouchEvent.TOUCH_MOVE, { die:true } ));
			this.addStep(new GestureStep(TuioEvent.NEW_FRAME, {goto:1} ));
		}
		
		public override function dispatchGestureEvent(target:DisplayObject, gsg:GestureStepSequence):void {
			trace("three finger move" + getTimer());
		}
		
	}
	
}