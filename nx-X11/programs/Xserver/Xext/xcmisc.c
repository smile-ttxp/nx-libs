/* $Xorg: xcmisc.c,v 1.4 2001/02/09 02:04:33 xorgcvs Exp $ */
/*

Copyright 1993, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/Xserver/Xext/xcmisc.c,v 3.7 2003/10/28 23:08:43 tsi Exp $ */

#define NEED_EVENTS
#define NEED_REPLIES
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "swaprep.h"
#include <nx-X11/extensions/xcmiscstr.h>
#include "modinit.h"

#if HAVE_STDINT_H
#include <stdint.h>
#elif !defined(UINT32_MAX)
#define UINT32_MAX 0xffffffffU
#endif

#if 0
static unsigned char XCMiscCode;
#endif

static void XCMiscResetProc(
    ExtensionEntry * /* extEntry */
);

static DISPATCH_PROC(ProcXCMiscDispatch);
static DISPATCH_PROC(ProcXCMiscGetVersion);
static DISPATCH_PROC(ProcXCMiscGetXIDList);
static DISPATCH_PROC(ProcXCMiscGetXIDRange);
static DISPATCH_PROC(SProcXCMiscDispatch);
static DISPATCH_PROC(SProcXCMiscGetVersion);
static DISPATCH_PROC(SProcXCMiscGetXIDList);
static DISPATCH_PROC(SProcXCMiscGetXIDRange);

void
XCMiscExtensionInit(void)
{
#if 0
    ExtensionEntry *extEntry;

    if ((extEntry = AddExtension(XCMiscExtensionName, 0, 0,
				ProcXCMiscDispatch, SProcXCMiscDispatch,
				XCMiscResetProc, StandardMinorOpcode)) != 0)
	XCMiscCode = (unsigned char)extEntry->base;
#else
    (void) AddExtension(XCMiscExtensionName, 0, 0,
			ProcXCMiscDispatch, SProcXCMiscDispatch,
			XCMiscResetProc, StandardMinorOpcode);
#endif

    DeclareExtensionSecurity(XCMiscExtensionName, TRUE);
}

/*ARGSUSED*/
static void
XCMiscResetProc (extEntry)
    ExtensionEntry	*extEntry;
{
}

static int
ProcXCMiscGetVersion(client)
    register ClientPtr client;
{
    xXCMiscGetVersionReply rep;

    REQUEST_SIZE_MATCH(xXCMiscGetVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = XCMiscMajorVersion;
    rep.minorVersion = XCMiscMinorVersion;
    if (client->swapped) {
	swaps(&rep.sequenceNumber);
	swaps(&rep.majorVersion);
	swaps(&rep.minorVersion);
    }
    WriteToClient(client, sizeof(xXCMiscGetVersionReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcXCMiscGetXIDRange(client)
    register ClientPtr client;
{
    xXCMiscGetXIDRangeReply rep;
    XID min_id, max_id;

    REQUEST_SIZE_MATCH(xXCMiscGetXIDRangeReq);
    GetXIDRange(client->index, FALSE, &min_id, &max_id);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.start_id = min_id;
    rep.count = max_id - min_id + 1;
    if (client->swapped) {
	swaps(&rep.sequenceNumber);
	swapl(&rep.start_id);
	swapl(&rep.count);
    }
    WriteToClient(client, sizeof(xXCMiscGetXIDRangeReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcXCMiscGetXIDList(client)
    register ClientPtr client;
{
    REQUEST(xXCMiscGetXIDListReq);
    xXCMiscGetXIDListReply rep;
    XID *pids;
    unsigned int count;

    REQUEST_SIZE_MATCH(xXCMiscGetXIDListReq);

    if (stuff->count > UINT32_MAX / sizeof(XID))
	    return BadAlloc;

    pids = (XID *)malloc(stuff->count * sizeof(XID));
    if (!pids)
    {
	return BadAlloc;
    }
    count = GetXIDList(client, stuff->count, pids);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = count;
    rep.count = count;
    if (client->swapped) {
	swaps(&rep.sequenceNumber);
	swapl(&rep.length);
	swapl(&rep.count);
    }
    WriteToClient(client, sizeof(xXCMiscGetXIDListReply), (char *)&rep);
    if (count)
    {
    	client->pSwapReplyFunc = (ReplySwapPtr) Swap32Write;
	WriteSwappedDataToClient(client, count * sizeof(XID), pids);
    }
    free(pids);
    return(client->noClientException);
}

static int
ProcXCMiscDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XCMiscGetVersion:
	return ProcXCMiscGetVersion(client);
    case X_XCMiscGetXIDRange:
	return ProcXCMiscGetXIDRange(client);
    case X_XCMiscGetXIDList:
	return ProcXCMiscGetXIDList(client);
    default:
	return BadRequest;
    }
}

static int
SProcXCMiscGetVersion(client)
    register ClientPtr	client;
{
    REQUEST(xXCMiscGetVersionReq);

    swaps(&stuff->length);
    REQUEST_SIZE_MATCH(xXCMiscGetVersionReq);
    swaps(&stuff->majorVersion);
    swaps(&stuff->minorVersion);
    return ProcXCMiscGetVersion(client);
}

static int
SProcXCMiscGetXIDRange(client)
    register ClientPtr	client;
{
    REQUEST(xReq);

    swaps(&stuff->length);
    return ProcXCMiscGetXIDRange(client);
}

static int
SProcXCMiscGetXIDList(client)
    register ClientPtr	client;
{
    REQUEST(xXCMiscGetXIDListReq);
    REQUEST_SIZE_MATCH(xXCMiscGetXIDListReq);

    swaps(&stuff->length);
    swapl(&stuff->count);
    return ProcXCMiscGetXIDList(client);
}

static int
SProcXCMiscDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XCMiscGetVersion:
	return SProcXCMiscGetVersion(client);
    case X_XCMiscGetXIDRange:
	return SProcXCMiscGetXIDRange(client);
    case X_XCMiscGetXIDList:
	return SProcXCMiscGetXIDList(client);
    default:
	return BadRequest;
    }
}
