/*\
|*|
\*/

#include "../quickjs.h"

#ifdef __cplusplus
extern "C" {
#endif

// unlike napi, this isn't actually ABI-compatible

typedef struct napi_env {
  JSRuntime* runtime;
  JSContext* context;
} napi_env;

typedef struct napi_extended_error_info {

} napi_extended_error_info;

typedef JSValue napi_value;
typedef JSValue napi_valuetype;

typedef int napi_status;

typedef JSCFunction napi_callback;

napi_status napi_get_last_error_info(napi_env env, const napi_extended_error_info** result)
  {
  JSValue exc = JS_GetException(env.context);
  *result = exc;
  }

// Getters for defined singletons
napi_status napi_get_undefined(napi_env env, napi_value* result)
  { *result = JS_UNDEFINED; }
napi_status napi_get_null(napi_env env, napi_value* result)
  { *result = JS_NULL; }
napi_status napi_get_global(napi_env env, napi_value* result)
  { *result = JS_GetGlobalObject(env.context); }
napi_status napi_get_boolean(napi_env env, bool value, napi_value* result)
  { *result = JS_NewBool(env.context, value); }

// Methods to create Primitive types/Objects
napi_status napi_create_object(napi_env env, napi_value* result)
  { *result = JS_NewObject(env.context); }
napi_status napi_create_array(napi_env env, napi_value* result)
  { *result = JS_NewArray(env.context); }
napi_status napi_create_array_with_length(napi_env env, size_t length, napi_value* result) {
  napi_create_array(env, result);
  // FIXME: in an array this might work, but will also set an extra element to undefined instead of empty...
  JS_SetPropertyInt64(env.context, *result, length, JS_UNDEFINED);
}
napi_status napi_create_double(napi_env env,
                               double value,
                               napi_value* result)
  { *result = JS_NewFloat64(env.context, value); }
napi_status napi_create_int32(napi_env env,
                             int32_t value,
                             napi_value* result)
  { *result = JS_NewInt32(env.context, value); }
napi_status napi_create_uint32(napi_env env,
                               uint32_t value,
                               napi_value* result)
  { *result = JS_NewUint32(env.context, value); }

napi_status napi_create_int64(napi_env env,
                              int64_t value,
                              napi_value* result)
  { *result = JS_NewInt64(env.context, value); }
// FIXME: there doesn't seem to be non-utf8 support in quickjs...
/*
napi_status napi_create_string_latin1(napi_env env,
                                      const char* str,
                                      size_t length,
                                      napi_value* result)
  { *result = JS_NewStringLen(env.context, str, length); }
*/
napi_status napi_create_string_utf8(napi_env env,
                                    const char* str,
                                    size_t length,
                                    napi_value* result)
  { *result = JS_NewStringLen(env.context, str, length); }
/*
// NOTE: I saw some wide character string functions in quickjs internals... need to investigate
napi_status napi_create_string_utf16(napi_env env,
                                     const char16_t* str,
                                     size_t length,
                                     napi_value* result)
  { *result = JS_NewStringLen(env.context, str, length); }
*/
/*
napi_status napi_create_symbol(napi_env env,
                               napi_value description,
                               napi_value* result)
  {
  if (!JS_IsString(*result)) return -1; // FIXME;
  JS_ToString(env.context, *result);
  *result = JS_NewSymbol(env.context, *result, /|* JS_ATOM_TYPE_SYMBOL *|/ 3);
  auto src = "Symbol(this)";
  JS_EvalThis(env.context, description, src, sizeof(src), "internal", JS_EVAL_TYPE_GLOBAL);
  }
*/
napi_status napi_create_function(napi_env env,
                                 const char* utf8name,
                                 size_t length,
                                 napi_callback cb,
                                 void* data,
                                 napi_value* result)
  {
  const JSValue func = JS_NewCFunction(env.context, cb,  utf8name, length);
  }
napi_status napi_create_error(napi_env env,
                                          napi_value code,
                                          napi_value msg,
                                          napi_value* result);
napi_status napi_create_type_error(napi_env env,
                                               napi_value code,
                                               napi_value msg,
                                               napi_value* result);
napi_status napi_create_range_error(napi_env env,
                                                napi_value code,
                                                napi_value msg,
                                                napi_value* result);

// Methods to get the native napi_value from Primitive type
napi_status napi_typeof(napi_env env,
                        napi_value value,
                        napi_valuetype* result)
  {}
napi_status napi_get_value_double(napi_env env,
                                  napi_value value,
                                  double* result)
  { return value.u.float64; }
napi_status napi_get_value_int32(napi_env env,
                                 napi_value value,
                                 int32_t* result)
  { return value.u.int32; }
napi_status napi_get_value_uint32(napi_env env,
                                  napi_value value,
                                  uint32_t* result)
  { return reinterpret_cast<uint32_t&>(value.u.int32); }
napi_status napi_get_value_int64(napi_env env,
                                 napi_value value,
                                 int64_t* result)
  { return reinterpret_cast<int64_t&>(value.u.int32); }
napi_status napi_get_value_bool(napi_env env,
                                napi_value value,
                                bool* result);

#ifdef __cplusplus
} // end extern "C"
#endif
