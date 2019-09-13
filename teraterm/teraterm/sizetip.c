/*
 * Copyright (C) 2008-2019 TeraTerm Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* original idea from PuTTY 0.60 windows/sizetip.c */

#include "teraterm.h"
#include "tttypes.h"
#include "ttlib.h"
#include "ttwinman.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "tipwin.h"

static TipWin *SizeTip;
static int tip_enabled = 0;

/*
 *	point ��
 *	�X�N���[������͂ݏo���Ă���ꍇ�A����悤�ɕ␳����
 *	NearestMonitor �� TRUE �̂Ƃ��A�ł��߂����j�^
 *	FALSE�̂Ƃ��A�}�E�X�̂��郂�j�^�Ɉړ�������
 *	�f�B�X�v���C�̒[���� FrameWidth(pixel) ��藣���悤�ɂ���
 */
static void FixPosFromFrame(POINT *point, int FrameWidth, BOOL NearestMonitor)
{
	if (HasMultiMonitorSupport()) {
		// �}���`���j�^���T�|�[�g����Ă���ꍇ
		HMONITOR hm;
		MONITORINFO mi;
		int ix, iy;

		// ���̍��W��ۑ����Ă���
		ix = point->x;
		iy = point->y;

		hm = MonitorFromPoint(*point, MONITOR_DEFAULTTONULL);
		if (hm == NULL) {
			if (NearestMonitor) {
				// �ł��߂����j�^�ɕ\������
				hm = MonitorFromPoint(*point, MONITOR_DEFAULTTONEAREST);
			} else {
				// �X�N���[������͂ݏo���Ă���ꍇ�̓}�E�X�̂��郂�j�^�ɕ\������
				GetCursorPos(point);
				hm = MonitorFromPoint(*point, MONITOR_DEFAULTTONEAREST);
			}
		}

		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hm, &mi);
		if (ix < mi.rcMonitor.left + FrameWidth) {
			ix = mi.rcMonitor.left + FrameWidth;
		}
		if (iy < mi.rcMonitor.top + FrameWidth) {
			iy = mi.rcMonitor.top + FrameWidth;
		}
		
		point->x = ix;
		point->y = iy;
	}
	else
	{
		// �}���`���j�^���T�|�[�g����Ă��Ȃ��ꍇ
		if (point->x < FrameWidth) {
			point->x = FrameWidth;
		}
		if (point->y < FrameWidth) {
			point->y = FrameWidth;
		}
	}
}

/* ���T�C�Y�p�c�[���`�b�v��\������
 *
 * �����F
 *   src        �E�B���h�E�n���h��
 *   cx, cy     �c�[���`�b�v�ɕ\������c���T�C�Y
 *   fwSide     ���T�C�Y���ɂǂ��̃E�B���h�E��͂񂾂�
 *   newX, newY ���T�C�Y��̍���̍��W
 *
 * ���ӁF Windows9x �ł͓��삵�Ȃ�
 */
void UpdateSizeTip(HWND src, int cx, int cy, UINT fwSide, int newX, int newY)
{
	TCHAR str[32];
	int tooltip_movable = 0;

	if (!tip_enabled)
		return;

	/* Generate the tip text */
	_stprintf_s(str, _countof(str), _T("%dx%d"), cx, cy);

	// �E�B���h�E�̉E�A�E���A����͂񂾏ꍇ�́A�c�[���`�b�v��������ɔz�u����B
	// �����ȊO�̓��T�C�Y��̍�����ɔz�u����B
	if (!(fwSide == WMSZ_RIGHT || fwSide == WMSZ_BOTTOMRIGHT || fwSide == WMSZ_BOTTOM)) {
		tooltip_movable = 1;
	}

	if (SizeTip == NULL) {
		RECT wr;
		POINT point;
		int w, h;

		// ������̏c���T�C�Y���擾����
		TipWinGetTextWidthHeight(src, str, &w, &h);

		// �E�B���h�E�̈ʒu���擾
		GetWindowRect(src, &wr);

		// sizetip���o���ʒu�́A�E�B���h�E����(X, Y)�ɑ΂��āA
		// (X, Y - ������̍��� - FRAME_WIDTH * 2) �Ƃ���B
		point.x = wr.left;
		point.y = wr.top - (h + FRAME_WIDTH * 2);
		FixPosFromFrame(&point, 16, FALSE);
		cx = point.x;
		cy = point.y;

		SizeTip = TipWinCreate(src, cx, cy, str);

		//OutputDebugPrintf("Created: (%d,%d)\n", cx, cy);

	} else {
		/* Tip already exists, just set the text */
		TipWinSetText(SizeTip, str);
		//SetWindowText(tip_wnd, str);

		//OutputDebugPrintf("Updated: (%d,%d)\n", cx, cy);

		// �E�B���h�E�̍��オ�ړ�����ꍇ
		if (tooltip_movable) {
			TipWinSetPos(SizeTip, newX + FRAME_WIDTH*2, newY + FRAME_WIDTH*2);
			//OutputDebugPrintf("Moved: (%d,%d)\n", newX, newY);
		}
	}
}

void EnableSizeTip(int bEnable)
{
	if (SizeTip && !bEnable) {
		TipWinDestroy(SizeTip);
		SizeTip = NULL;
	}

	tip_enabled = bEnable;
}
