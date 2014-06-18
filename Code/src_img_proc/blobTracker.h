
/****************************************
 * 	blobTracker.h
 *
 *	<description>
 *
 *  Created on: Oct, 2013
 *      Author: dbs
 *     Project: S.R.
 ****************************************/


#ifndef BLOBTRACKER_H_
#define BLOBTRACKER_H_

#include "datatypes.h"

#include <cv.h>


typedef struct Blob
{
	CvRect rect;
	CvPoint center;
	CvRect trackrect;
	CvBox2D trackbox;
	double area;
	int seenblob;
	int trackable;
} Blob;

typedef struct BlobTracker
{
	struct BlobTracker *next;

	int id;
	int disable;
	char name[32];

	int vmin, vmax;
	int smin, smax;
	int numbins;
	CvHistogram *hist;

	int numblobs;
	double minArea, maxArea;
	double bbMinArea, bbMaxArea;
	Blob *blobs;

	CvScalar txtcolor, icolor;
} BlobTracker;


void trackBlobs(BlobTracker *bt, IplImage* frame, bit trackAll);
BlobTracker *createBlobTracker(BlobTracker **prevbt, int id, char *name,
							   int numblobs, IplImage* frame, CvRect *selection,
							   BlobTracker *btval);
int loadBlobTrackers(BlobTracker **bt, char *filename);
int saveBlobTrackers(BlobTracker *bt, char *filename);
BlobTracker *getBlobTracker(BlobTracker *bt, int btid);
int getBlobPos(BlobTracker *bt, int btid, int bbid, CvPoint *center);
int initBlobTrackers(CvSize frameSize);
void closeBlobTrackers();



#endif /* BLOBTRACKER_H_ */
