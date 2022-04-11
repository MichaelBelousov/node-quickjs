/*\
|*|
\*/

#include "./js_native_api_types_quickjs.h"
#include "../quickjs.h"
#include "./vendor/js_native_api.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

napi_status napi_get_last_error_info(napi_env env, const napi_extended_error_info** result)
  {
  JSValue exc = JS_GetException(env->context);
  *result = exc;
  }

// Getters for defined singletons
napi_status napi_get_undefined(napi_env env, napi_value* result)
  { **result = JS_UNDEFINED; }
napi_status napi_get_null(napi_env env, napi_value* result)
  { **result = JS_NULL; }
napi_status napi_get_global(napi_env env, napi_value* result)
  { **result = JS_GetGlobalObject(env->context); }
napi_status napi_get_boolean(napi_env env, bool value, napi_value* result)
  { **result = JS_NewBool(env->context, value); }

// Methods to create Primitive types/Objects
napi_status napi_create_object(napi_env env, napi_value* result)
  { **result = JS_NewObject(env->context); }
napi_status napi_create_array(napi_env env, napi_value* result)
  { **result = JS_NewArray(env->context); }
napi_status napi_create_array_with_length(napi_env env, size_t length, napi_value* result) {
  napi_create_array(env, result);
  // FIXME: in an array this might work, but will also set an extra element to undefined instead of empty...
  JS_SetPropertyInt64(env->context, **result, length, JS_UNDEFINED);
}
napi_status napi_create_double(napi_env env,
                               double value,
                               napi_value* result)
  { **result = JS_NewFloat64(env->context, value); }
napi_status napi_create_int32(napi_env env,
                             int32_t value,
                             napi_value* result)
  { **result = JS_NewInt32(env->context, value); }
napi_status napi_create_uint32(napi_env env,
                               uint32_t value,
                               napi_value* result)
  { **result = JS_NewUint32(env->context, value); }

napi_status napi_create_int64(napi_env env,
                              int64_t value,
                              napi_value* result)
  { **result = JS_NewInt64(env->context, value); }
// FIXME: there doesn't seem to be non-utf8 support in quickjs...
/*
napi_status napi_create_string_latin1(napi_env env,
                                      const char* str,
                                      size_t length,
                                      napi_value* result)
  { **result = JS_NewStringLen(env->context, str, length); }
*/
napi_status napi_create_string_utf8(napi_env env,
                                    const char* str,
                                    size_t length,
                                    napi_value* result)
  { **result = JS_NewStringLen(env->context, str, length); }
/*
// NOTE: I saw some wide character string functions in quickjs internals... need to investigate
napi_status napi_create_string_utf16(napi_env env,
                                     const char16_t* str,
                                     size_t length,
                                     napi_value* result)
  { **result = JS_NewStringLen(env->context, str, length); }
*/
/*
napi_status napi_create_symbol(napi_env env,
                               napi_value description,
                               napi_value* result)
  {
  if (!JS_IsString(*result)) return -1; // FIXME;
  JS_ToString(env->context, *result);
  **result = JS_NewSymbol(env->context, *result, /|* JS_ATOM_TYPE_SYMBOL *|/ 3);
  JSValue src = "Symbol(this)";
  JS_EvalThis(env->context, description, src, sizeof(src), "internal", JS_EVAL_TYPE_GLOBAL);
  }
*/

typedef struct NapiWrappedQjsFuncData {
  JSCFunction qjsFunc;
} NapiWrappedQjsFuncData ;

/** a napi_callback that redirects to a qjs function */
napi_status qjs_wrapped_napi_callback(napi_env env,
                                      napi_callback_info cbinfo)
  {
  size_t argc;
  napi_value* argv;
  napi_value this_arg;
  void* data;
  napi_status status;
  if (napi_ok != (status =
    napi_get_cb_info(env, cbinfo, &argc, &argv, &this_arg, &data)))
    return status;
  NapiWrappedQjsFuncData* qjsFuncData = (NapiWrappedQjsFuncData*)data;
  qjsFuncData->qjsFunc(env->context, *this_arg, argc, *argv);
  }

// FIXME: initialize at module init time
JSClassID qjs_func_from_napi_data_class_id;

typedef struct NapiCallbackPayload {
  napi_callback cb;
  void* data;
} NapiCallbackPayload;

/** a qjs cfunction containing a napi_callback to use for invoking */
JSValue qjs_func_from_napi_callback(
  JSContext *ctx,
  JSValueConst this_val,
  int argc,
  JSValueConst *argv,
  int _magic,
  JSValue* func_data
) {
  NapiCallbackPayload* payload = (NapiCallbackPayload*)JS_GetOpaque(func_data[0], qjs_func_from_napi_data_class_id);
}

napi_status napi_create_function(napi_env env,
                                 // FIXME: quickjs ignores name for cfunc_data functions
                                 const char* _utf8name,
                                 size_t length,
                                 napi_callback cb,
                                 void* data,
                                 napi_value* result) {
  JSValue val = JS_NewObject(env->context);
  // FIXME: install a finalizer to free this
  NapiCallbackPayload* payload = (NapiCallbackPayload*)malloc(sizeof(NapiCallbackPayload));
  payload->cb = cb;
  payload->data = data;
  JS_SetOpaque(val, payload);
  **result = JS_NewCFunctionData(
    env->context,
    &qjs_func_from_napi_callback,
    length,
    JS_CFUNC_generic,
    1,
    &val
  );
}

napi_status napi_create_error(napi_env env,
                              napi_value code,
                              napi_value msg,
                              napi_value* result) {
  **result = JS_NewError(env->context);
  int status;
  if (napi_ok != (status = JS_SetPropertyStr(env->context, **result, "code", *code)))
    // FIXME: why am I comparing a quickjs error code to napi one, need to translate them
    return (napi_status)status;
  if (napi_ok != (status = JS_SetPropertyStr(env->context, **result, "message", *msg)))
    // FIXME: why am I comparing a quickjs error code to napi one, need to translate them
    return (napi_status)status;
}
napi_status napi_create_type_error(napi_env env,
                                   napi_value code,
                                   napi_value msg,
                                   napi_value* result) {
  **result = JS_NewTypeError(env->context, code, msg);
  //JS_NewObjectProtoClass(env->context, env->context->native_error_proto[JS_TYPE_ERROR], JS_CLASS_ERROR);
}
napi_status napi_create_range_error(napi_env env,
                                    napi_value code,
                                    napi_value msg,
                                    napi_value* result) {
  **result = JS_NewRangeError(env->context, code, msg);
}

// Methods to get the native napi_value from Primitive type
napi_status napi_typeof(napi_env env,
                        napi_value value,
                        napi_valuetype* result) {
  //JS_typeof
}
napi_status napi_get_value_double(napi_env env,
                                  napi_value value,
                                  double* result)
  { *result = value->u.float64; }
napi_status napi_get_value_int32(napi_env env,
                                 napi_value value,
                                 int32_t* result)
  { *result = value->u.int32; }
napi_status napi_get_value_uint32(napi_env env,
                                  napi_value value,
                                  uint32_t* result)
  { *result = (uint32_t)value->u.float64; }
napi_status napi_get_value_int64(napi_env env,
                                 napi_value value,
                                 int64_t* result)
  { *result = (int64_t)value->u.float64; }
napi_status napi_get_value_bool(napi_env env,
                                napi_value value,
                                bool* result)
  { *result = (bool)value->u.int32; }

#ifdef __cplusplus
} // end extern "C"
#endif
