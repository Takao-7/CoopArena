// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef TEST_testBPLibrary_generated_h
#error "testBPLibrary.generated.h already included, missing '#pragma once' in testBPLibrary.h"
#endif
#define TEST_testBPLibrary_generated_h

#define CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(exectestSampleFunction) \
	{ \
		P_GET_PROPERTY(UFloatProperty,Z_Param_Param); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(float*)Z_Param__Result=UtestBPLibrary::testSampleFunction(Z_Param_Param); \
		P_NATIVE_END; \
	}


#define CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(exectestSampleFunction) \
	{ \
		P_GET_PROPERTY(UFloatProperty,Z_Param_Param); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(float*)Z_Param__Result=UtestBPLibrary::testSampleFunction(Z_Param_Param); \
		P_NATIVE_END; \
	}


#define CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUtestBPLibrary(); \
	friend struct Z_Construct_UClass_UtestBPLibrary_Statics; \
public: \
	DECLARE_CLASS(UtestBPLibrary, UBlueprintFunctionLibrary, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/test"), NO_API) \
	DECLARE_SERIALIZER(UtestBPLibrary)


#define CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_INCLASS \
private: \
	static void StaticRegisterNativesUtestBPLibrary(); \
	friend struct Z_Construct_UClass_UtestBPLibrary_Statics; \
public: \
	DECLARE_CLASS(UtestBPLibrary, UBlueprintFunctionLibrary, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/test"), NO_API) \
	DECLARE_SERIALIZER(UtestBPLibrary)


#define CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UtestBPLibrary(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UtestBPLibrary) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UtestBPLibrary); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UtestBPLibrary); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UtestBPLibrary(UtestBPLibrary&&); \
	NO_API UtestBPLibrary(const UtestBPLibrary&); \
public:


#define CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UtestBPLibrary(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UtestBPLibrary(UtestBPLibrary&&); \
	NO_API UtestBPLibrary(const UtestBPLibrary&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UtestBPLibrary); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UtestBPLibrary); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UtestBPLibrary)


#define CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_PRIVATE_PROPERTY_OFFSET
#define CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_25_PROLOG
#define CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_PRIVATE_PROPERTY_OFFSET \
	CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_RPC_WRAPPERS \
	CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_INCLASS \
	CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_PRIVATE_PROPERTY_OFFSET \
	CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_RPC_WRAPPERS_NO_PURE_DECLS \
	CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_INCLASS_NO_PURE_DECLS \
	CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h_28_ENHANCED_CONSTRUCTORS \
static_assert(false, "Unknown access specifier for GENERATED_BODY() macro in class testBPLibrary."); \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID CoopArena_Plugins_test_Source_test_Public_testBPLibrary_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
