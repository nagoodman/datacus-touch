/**
 * This is a generated class and is not intended for modification.  To customize behavior
 * of this service wrapper you may modify the generated sub-class of this class - Create_Query.as.
 */
package services.create_query
{
import com.adobe.fiber.core.model_internal;
import com.adobe.fiber.services.wrapper.HTTPServiceWrapper;
import mx.rpc.AbstractOperation;
import mx.rpc.AsyncToken;
import mx.rpc.http.HTTPMultiService;
import mx.rpc.http.Operation;
import mx.core.FlexGlobals;


[ExcludeClass]
internal class _Super_Create_Query extends com.adobe.fiber.services.wrapper.HTTPServiceWrapper
{

    // Constructor
    public function _Super_Create_Query()
    {
        // initialize service control
        _serviceControl = new mx.rpc.http.HTTPMultiService();
         var operations:Array = new Array();
         var operation:mx.rpc.http.Operation;
         var argsArray:Array;
		 
         operation = new mx.rpc.http.Operation(null, "CreateQuery");
         operation.url = "http://localhost:8080/saiku/json/saiku/admin/query/chartdata/";
         operation.method = "POST";
         argsArray = new Array("connection","cube","catalog","schema");
         operation.argumentNames = argsArray;         
         operation.contentType = "application/x-www-form-urlencoded";
         operation.resultType = Object;
         operations.push(operation);

         _serviceControl.operationList = operations;  



         model_internal::initialize();
    }

    /**
      * This method is a generated wrapper used to call the 'CreateQuery' operation. It returns an mx.rpc.AsyncToken whose 
      * result property will be populated with the result of the operation when the server response is received. 
      * To use this result from MXML code, define a CallResponder component and assign its token property to this method's return value. 
      * You can then bind to CallResponder.lastResult or listen for the CallResponder.result or fault events.
      *
      * @see mx.rpc.AsyncToken
      * @see mx.rpc.CallResponder 
      *
      * @return an mx.rpc.AsyncToken whose result property will be populated with the result of the operation when the server response is received.
      */
    public function CreateQuery(connection:String, cube:String, catalog:String, schema:String) : mx.rpc.AsyncToken
    {
        var _internal_operation:mx.rpc.AbstractOperation = _serviceControl.getOperation("CreateQuery");
        var _internal_token:mx.rpc.AsyncToken = _internal_operation.send(connection,cube,catalog,schema) ;

        return _internal_token;
    }
     
}

}
