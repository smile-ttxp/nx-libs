
/************************************************************

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Copyright 1989 by Hewlett-Packard Company, Palo Alto, California.

			All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Hewlett-Packard not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HEWLETT-PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

/***********************************************************************
 *
 * Extension function to grab a button on an extension device.
 *
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <nx-X11/X.h>	/* for inputstr.h    */
#include <nx-X11/Xproto.h>	/* Request macro     */
#include "inputstr.h"	/* DeviceIntPtr      */
#include "windowstr.h"	/* window structure  */
#include <nx-X11/extensions/XI.h>
#include <nx-X11/extensions/XIproto.h>
#include "exevents.h"
#include "extnsionst.h"
#include "extinit.h"	/* LookupDeviceIntRec */
#include "exglobals.h"

#include "grabdev.h"
#include "grabdevb.h"

/***********************************************************************
 *
 * Handle requests from clients with a different byte order.
 *
 */

int
SProcXGrabDeviceButton(register ClientPtr client)
{
    REQUEST(xGrabDeviceButtonReq);
    swaps(&stuff->length);
    REQUEST_AT_LEAST_SIZE(xGrabDeviceButtonReq);
    swapl(&stuff->grabWindow);
    swaps(&stuff->modifiers);
    swaps(&stuff->event_count);
    REQUEST_FIXED_SIZE(xGrabDeviceButtonReq,
                      stuff->event_count * sizeof(CARD32));
    SwapLongs((CARD32 *) (&stuff[1]), stuff->event_count);

    return (ProcXGrabDeviceButton(client));
}

/***********************************************************************
 *
 * Grab a button on an extension device.
 *
 */

int
ProcXGrabDeviceButton(ClientPtr client)
{
    int ret;
    DeviceIntPtr dev;
    DeviceIntPtr mdev;
    XEventClass *class;
    struct tmask tmp[EMASKSIZE];

    REQUEST(xGrabDeviceButtonReq);
    REQUEST_AT_LEAST_SIZE(xGrabDeviceButtonReq);

    if (stuff->length !=
	(sizeof(xGrabDeviceButtonReq) >> 2) + stuff->event_count) {
	SendErrorToClient(client, IReqCode, X_GrabDeviceButton, 0, BadLength);
	return Success;
    }

    dev = LookupDeviceIntRec(stuff->grabbed_device);
    if (dev == NULL) {
	SendErrorToClient(client, IReqCode, X_GrabDeviceButton, 0, BadDevice);
	return Success;
    }
    if (stuff->modifier_device != UseXKeyboard) {
	mdev = LookupDeviceIntRec(stuff->modifier_device);
	if (mdev == NULL) {
	    SendErrorToClient(client, IReqCode, X_GrabDeviceButton, 0,
			      BadDevice);
	    return Success;
	}
	if (mdev->key == NULL) {
	    SendErrorToClient(client, IReqCode, X_GrabDeviceButton, 0,
			      BadMatch);
	    return Success;
	}
    } else
	mdev = (DeviceIntPtr) LookupKeyboardDevice();

    class = (XEventClass *) (&stuff[1]);	/* first word of values */

    if ((ret = CreateMaskFromList(client, class,
				  stuff->event_count, tmp, dev,
				  X_GrabDeviceButton)) != Success)
	return Success;
    ret = GrabButton(client, dev, stuff->this_device_mode,
		     stuff->other_devices_mode, stuff->modifiers, mdev,
		     stuff->button, stuff->grabWindow, stuff->ownerEvents,
		     (Cursor) 0, (Window) 0, tmp[stuff->grabbed_device].mask);

    if (ret != Success)
	SendErrorToClient(client, IReqCode, X_GrabDeviceButton, 0, ret);
    return (Success);
}
