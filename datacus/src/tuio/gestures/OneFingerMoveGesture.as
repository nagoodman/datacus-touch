package tuio.gestures {
	
	import flash.display.DisplayObject;
	import flash.utils.getTimer;
	import tuio.TuioContainer;
	import tuio.TuioEvent;
	import tuio.TouchEvent;
	
	public class OneFingerMoveGesture extends Gesture {
		
		public function OneFingerMoveGesture() {
			this.addStep(new GestureStep(TouchEvent.TOUCH_DOWN, { tuioContainerAlias:"A", targetAlias:"A" } ));
			this.addStep(new GestureStep(TouchEvent.TOUCH_MOVE, { die:true, tuioContainerAlias:"!B", targetAlias:"A" } ));
			this.addStep(new GestureStep(TouchEvent.TOUCH_UP, { die:true, tuioContainerAlias:"A" } ));
			this.addStep(new GestureStep(TouchEvent.TOUCH_MOVE, { tuioContainerAlias:"A", targetAlias:"A", goto:2 } ));
		}
		
		public override function dispatchGestureEvent(target:DisplayObject, gsg:GestureStepSequence):void {
			trace("one finger move" + getTimer());
		}
		
	}
	
}