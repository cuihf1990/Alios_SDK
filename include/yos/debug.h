/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

#define SHORT_FILE __FILENAME__

#define debug_print_assert(A,B,C,D,E,F)

// ==== BRANCH PREDICTION & EXPRESSION EVALUATION ====
#if( !defined( unlikely ) )
//#define unlikely( EXPRESSSION )     __builtin_expect( !!(EXPRESSSION), 0 )
#define unlikely( EXPRESSSION )     !!(EXPRESSSION)
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    check
    @abstract   Check that an expression is true (non-zero).
    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number,
    function name, etc.) using the default debugging output method.

    Code inside check() statements is not compiled into production builds.
*/

#if( !defined( check ) )
#define check( X )                                                                                  \
        do                                                                                              \
        {                                                                                               \
            if( unlikely( !(X) ) )                                                                      \
            {                                                                                           \
                debug_print_assert( 0, #X, NULL, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );             \
            }                                                                                           \
                                                                                                        \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    check_string
    @abstract   Check that an expression is true (non-zero) with an explanation.
    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number,
    function name, etc.) using the default debugging output method.

    Code inside check() statements is not compiled into production builds.
*/

#if( !defined( check_string ) )
#define check_string( X, STR )                                                                                  \
        do                                                                                              \
        {                                                                                               \
            if( unlikely( !(X) ) )                                                                      \
            {                                                                                           \
                debug_print_assert( 0, #X, STR, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );              \
                YOS_ASSERTION_FAIL_ACTION();                                                           \
            }                                                                                           \
                                                                                                        \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require
    @abstract   Requires that an expression evaluate to true.
    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number,
    function name, etc.) using the default debugging output method then jumps to a label.
*/

#if( !defined( require ) )
#define require( X, LABEL )                                                                             \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                debug_print_assert( 0, #X, NULL, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_string
    @abstract   Requires that an expression evaluate to true with an explanation.
    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number,
    function name, etc.) and a custom explanation string using the default debugging output method then jumps to a label.
*/

#if( !defined( require_string ) )
#define require_string( X, LABEL, STR )                                                                 \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                debug_print_assert( 0, #X, STR, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );                  \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_quiet
    @abstract   Requires that an expression evaluate to true.
    @discussion

    If expression evalulates to false, this jumps to a label. No debugging information is printed.
*/

#if( !defined( require_quiet ) )
#define require_quiet( X, LABEL )                                                                       \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr
    @abstract   Require that an error code is noErr (0).
    @discussion

    If the error code is non-0, this prints debugging information (actual expression string, file, line number,
    function name, etc.) using the default debugging output method then jumps to a label.
*/

#if( !defined( require_noerr ) )
#define require_noerr( ERR, LABEL )                                                                     \
        do                                                                                                  \
        {                                                                                                   \
            int        localErr;                                                                       \
                                                                                                            \
            localErr = (int)(ERR);                                                                     \
            if( unlikely( localErr != 0 ) )                                                                 \
            {                                                                                               \
                debug_print_assert( localErr, NULL, NULL, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );        \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_string
    @abstract   Require that an error code is noErr (0).
    @discussion

    If the error code is non-0, this prints debugging information (actual expression string, file, line number,
    function name, etc.), and a custom explanation string using the default debugging output method using the
    default debugging output method then jumps to a label.
*/

#if( !defined( require_noerr_string ) )
#define require_noerr_string( ERR, LABEL, STR )                                                         \
        do                                                                                                  \
        {                                                                                                   \
            int        localErr;                                                                       \
                                                                                                            \
            localErr = (int)(ERR);                                                                     \
            if( unlikely( localErr != 0 ) )                                                                 \
            {                                                                                               \
                debug_print_assert( localErr, NULL, STR, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );         \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_action_string
    @abstract   Require that an error code is noErr (0).
    @discussion

    If the error code is non-0, this prints debugging information (actual expression string, file, line number,
    function name, etc.), and a custom explanation string using the default debugging output method using the
    default debugging output method then executes an action and jumps to a label.
*/

#if( !defined( require_noerr_action_string ) )
#define require_noerr_action_string( ERR, LABEL, ACTION, STR )                                          \
        do                                                                                                  \
        {                                                                                                   \
            int        localErr;                                                                       \
                                                                                                            \
            localErr = (int)(ERR);                                                                     \
            if( unlikely( localErr != 0 ) )                                                                 \
            {                                                                                               \
                debug_print_assert( localErr, NULL, STR, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );         \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_quiet
    @abstract   Require that an error code is noErr (0).
    @discussion

    If the error code is non-0, this jumps to a label. No debugging information is printed.
*/

#if( !defined( require_noerr_quiet ) )
#define require_noerr_quiet( ERR, LABEL )                                                               \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( (ERR) != 0 ) )                                                                    \
            {                                                                                               \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_action
    @abstract   Require that an error code is noErr (0) with an action to execute otherwise.
    @discussion

    If the error code is non-0, this prints debugging information (actual expression string, file, line number,
    function name, etc.) using the default debugging output method then executes an action and jumps to a label.
*/

#if( !defined( require_noerr_action ) )
#define require_noerr_action( ERR, LABEL, ACTION )                                                      \
        do                                                                                                  \
        {                                                                                                   \
            int        localErr;                                                                       \
                                                                                                            \
            localErr = (int)(ERR);                                                                     \
            if( unlikely( localErr != 0 ) )                                                                 \
            {                                                                                               \
                debug_print_assert( localErr, NULL, NULL, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );        \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_action_quiet
    @abstract   Require that an error code is noErr (0) with an action to execute otherwise.
    @discussion

    If the error code is non-0, this executes an action and jumps to a label. No debugging information is printed.
*/

#if( !defined( require_noerr_action_quiet ) )
#define require_noerr_action_quiet( ERR, LABEL, ACTION )                                                \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( (ERR) != 0 ) )                                                                    \
            {                                                                                               \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_action
    @abstract   Requires that an expression evaluate to true with an action to execute otherwise.
    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number,
    function name, etc.) using the default debugging output method then executes an action and jumps to a label.
*/

#if( !defined( require_action ) )
#define require_action( X, LABEL, ACTION )                                                              \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                debug_print_assert( 0, #X, NULL, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );                 \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_action_string
    @abstract   Requires that an expression evaluate to true with an explanation and action to execute otherwise.
    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number,
    function name, etc.) and a custom explanation string using the default debugging output method then executes an
    action and jumps to a label.
*/

#if( !defined( require_action_string ) )
#define require_action_string( X, LABEL, ACTION, STR )                                                  \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                debug_print_assert( 0, #X, STR, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );                  \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_action_quiet
    @abstract   Requires that an expression evaluate to true with an action to execute otherwise.
    @discussion

    If expression evalulates to false, this executes an action and jumps to a label. No debugging information is printed.
*/

#if( !defined( require_action_quiet ) )
#define require_action_quiet( X, LABEL, ACTION )                                                        \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

#if defined(__cplusplus)
}
#endif

#endif
