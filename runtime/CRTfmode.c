/*
 * CRTfmode.c
 *
 * Sets _CRT_fmode to be zero, which will cause _mingw32_init_fmode to leave
 * all file modes in their default state (basically text mode).
 *
 * This file is part of the Mingw32 package.
 *
 * THIS FILE IS IN THE PUBLIC DOMAIN.
 *
 * Contributers:
 *   Created by Colin Peters <colin@fu.is.saga-u.ac.jp>
 *
 * $Revision: 1.2 $
 * $Author: earnie $
 * $Date: 2001-06-05 00:26:30 $
 *
 */

unsigned int _CRT_fmode = 0;
