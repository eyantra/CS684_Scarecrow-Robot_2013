
/****************************************
 * 	blobTracker.c
 *
 *	<description>
 *
 *  Created on: Oct, 2013
 *      Author: dbs
 *     Project: S.R.
 ****************************************/


#include "blobTracker.h"

#include <stdio.h>
#include <stdlib.h>
#include <cv.h>


//#define SHOW_DEBUG_WIN

// Global variables of blob tracking functions
IplImage *hsv=0, *hue=0, *mask=0, *backproject=0;
CvFont font;

BlobTracker *getBlobTracker(BlobTracker *bt, int btid)
{
	while (bt)
	{
		if (bt->id == btid)
			return bt;
		bt = bt->next;
	}
	return 0;
}

int getBlobPos(BlobTracker *bt, int btid, int bbid, CvPoint *center)
{
	Blob *bb;

	while (bt)
	{
		if (bt->id == btid)
		{
			bb = bt->blobs;
			bb += bbid;
			if (!bb->trackable)
				return 0;
			*center = bb->center;
			return 1;
		}
		bt = bt->next;
	}
	return 0;
}

//if any blob is lost, entire set is refreshed
//passed image is destroyed
int findBlobs(BlobTracker *bt, IplImage *image)
{
	int i, j, k;
	int numblobs;
	Blob *bb;
/////////////////////////////////////////UNWANTED
	IplImage *gg = cvCreateImage(cvGetSize(hue), 8, 3);
	///////////////////////////////////////

	//allocate memory for contours
	CvMemStorage *contourStorage = cvCreateMemStorage(0);
	CvSeq *contour = NULL;

	struct ContArray
	{
		double area;
		CvSeq *contour;
	} *contArray, *contPtr, *largeContPtr;

	if (!contourStorage)
		return -1;

	for (i=bt->numblobs, bb=bt->blobs; i; --i, ++bb)
		bb->seenblob = 0;

	//find the contours
	numblobs = cvFindContours(image, contourStorage, &contour, sizeof(CvContour),
							  CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));

	//if no blobs found, return 0
	if (!numblobs)
		goto exit;

	//CvMoments moment;
	double area;
	double maxArea = bt->bbMaxArea;
	double minArea = bt->bbMinArea;

	//allocate memory to store contour area & contour pointer
	if (!(contArray=malloc(sizeof(struct ContArray)*numblobs)))
	{
		numblobs = -1;
		goto exit;
	}
	contPtr = contArray;

	//loop for each contour
	for (numblobs=0; contour; contour=contour->h_next)
	{
		//get contour area
		area = cvContourArea(contour, CV_WHOLE_SEQ, 0);

CvRect r=cvBoundingRect(contour, 0);
cvRectangle(gg,cvPoint(r.x,r.y),cvPoint(r.x+r.width,r.y+r.height),CV_RGB(0,255,0),1,8,0);
		//check if it is in required range
		if (area < minArea)
			continue;
		if (maxArea)
			if (area > maxArea)
				continue;
		contPtr->area = area;
		contPtr->contour = contour;
		++contPtr;
		++numblobs;
	}

#ifdef SHOW_DEBUG_WIN
	cvShowImage( "q", gg );
#endif

	//if no valid blobs, return 0
	if (!numblobs)
	{
		free(contArray);
		goto exit;
	}

	//loop for finding each blob
	for (i=bt->numblobs, bb=bt->blobs, j=numblobs; i&&j; --i, --j, ++bb)
	{
		largeContPtr = contArray;
		if (numblobs > 1)
		{
			contPtr = contArray+1;
			for (k=numblobs; k; --k, ++contPtr)
			{
				if (contPtr->area > largeContPtr->area)
					largeContPtr = contPtr;
			}
		}
		bb->area = largeContPtr->area;
		bb->rect = cvBoundingRect(largeContPtr->contour, 0);

		//if (!bb->trackable)
		bb->trackrect = bb->rect;
		bb->seenblob = 1;
		largeContPtr->area = 0;
	}
	free(contArray);
exit:
	cvReleaseImage(&gg);
	cvReleaseMemStorage(&contourStorage);

	return numblobs;
}

//#define SHOW_DEBUG_WIN

void trackBlobs(BlobTracker *bt, IplImage* frame, bit trackAll)
{
	int i, j;
	Blob *bb;
	CvConnectedComp trackcomp;
	char txt[40];

	cvCvtColor(frame, hsv, CV_BGR2HSV);

	while(bt)
	{
		cvInRangeS(hsv, cvScalar(0, bt->smin, bt->vmin, 0),
		           cvScalar(180, bt->smax, bt->vmax, 0), mask);
		cvSplit(hsv, hue, 0, 0, 0);
		cvCalcBackProject(&hue, backproject, bt->hist);
		cvAnd(backproject, mask, backproject, 0);

#ifdef SHOW_DEBUG_WIN
		if (bt->id==1)
			cvShowImage( "Ball",backproject);
		if (bt->id==2)
			cvShowImage( "Goal Post",backproject);
#endif

		for (i=bt->numblobs, bb=bt->blobs; i; --i, ++bb)
		{
			if (!bb->trackable)
			{
				cvCopy(backproject, mask, NULL);
				findBlobs(bt, mask);
				printf("find bb %s\n", bt->name);
				break;
			}
		}
		for (i=bt->numblobs, j=0, bb=bt->blobs; i; --i, ++j, ++bb)
		{
			if (bb->seenblob)
			{
				cvCamShift(backproject, bb->trackrect,
					       cvTermCriteria(CV_TERMCRIT_EPS|CV_TERMCRIT_ITER, 10, 1),
					       &trackcomp, &bb->trackbox);
				bb->trackrect = trackcomp.rect;
				bb->trackable = 1;
				bb->center = cvPoint(trackcomp.rect.x+trackcomp.rect.width/2,
									 trackcomp.rect.y+trackcomp.rect.height/2);
				if (trackcomp.area < bt->minArea)
				{
					printf("%s - area(%g) less than min(%g)\n", bt->name, trackcomp.area, bt->minArea);
					bb->trackable = 0;
				}
				if (bt->maxArea && trackcomp.area>bt->maxArea)
					bb->trackable = 0;
				if (bb->trackable)
				{
					if(!frame->origin)
						bb->trackbox.angle = -bb->trackbox.angle;
					cvCircle(frame, bb->center, 5, bt->icolor, -1, 8, 0);
					cvEllipseBox(frame, bb->trackbox, bt->icolor, 3, CV_AA, 0);
				//	if (font)///////////////////////////////////////////
					{
						if (bt->numblobs > 1)
							sprintf(txt, "%s[%d]: (%d, %d)", bt->name, j, bb->center.x, bb->center.y);
						else
							sprintf(txt, "%s: (%d, %d)", bt->name, bb->center.x, bb->center.y);
						cvPutText(frame, txt,
								  cvPoint(trackcomp.rect.x, trackcomp.rect.y), &font, bt->txtcolor);
					}
				}
			}
		}
		if (!trackAll)
			return;
		bt = bt->next;
	}
}

BlobTracker *createBlobTracker(BlobTracker **prevbt, int id, char *name,
							   int numblobs, IplImage* frame, CvRect *selection,
							   BlobTracker *btval)
{
	int i;
	float *bins;
	float hrangesArr[] = {0, 180};
	float* hranges = hrangesArr;
	BlobTracker *bt, *newbt;
	Blob *bb;
	CvConnectedComp trackcomp;
	CvHistogram *hist;
	float maxval = 0.0f;

	if (btval)
		i = btval->numbins;
	else
		i = 16;
	if (!(newbt=(BlobTracker *)malloc(sizeof(BlobTracker)+(sizeof(Blob)*numblobs)
									  +sizeof(CvHistogram)+(sizeof(float)*i))))
		return NULL;
	if (btval)
		*newbt = *btval;
	else
	{
		newbt->id = id;
		newbt->disable = 0;
		newbt->numblobs = numblobs;
		newbt->maxArea=newbt->bbMaxArea=0;
		newbt->smax=newbt->vmax=256;
		newbt->smin = 0.4*256;
		newbt->vmin = 0.2*256;
		newbt->numbins = 16;
		newbt->icolor = CV_RGB(0, 255, 0);
		newbt->txtcolor = CV_RGB(0, 0, 255);
	}
	newbt->blobs = bb = (Blob *)(newbt+1);
	newbt->hist = hist = (CvHistogram *)(bb+numblobs);
	bins = (float *)(newbt->hist+1);
	for(i=newbt->numblobs; i; --i, ++bb)
		bb->seenblob=bb->trackable=0;
	cvMakeHistHeaderForArray(1, &(newbt->numbins), newbt->hist, bins, &hranges, 1);
	if (!*prevbt)
		*prevbt = newbt;
	else
	{
		bt = *prevbt;
		while (bt->next)
			bt = bt->next;
		bt->next = newbt;
	}

	cvCvtColor(frame, hsv, CV_BGR2HSV);
	cvInRangeS(hsv, cvScalar(0, newbt->smin, newbt->vmin, 0),
		       cvScalar(180, newbt->smax, newbt->vmax, 0), mask);
	cvSplit(hsv, hue, 0, 0, 0);
	cvSetImageROI(hue, *selection);
	cvSetImageROI(mask, *selection);
	cvCalcHist(&hue, hist, 0, mask);
	cvGetMinMaxHistValue(hist, 0, &maxval, 0, 0);
	cvConvertScale(hist->bins, hist->bins, maxval ? 255.0 / maxval : 0.0, 0);
	cvResetImageROI(hue);
	cvResetImageROI(mask);

	newbt->numblobs = numblobs;
	if (!btval || btval->minArea==0)
	{
		cvCalcBackProject(&hue, backproject, hist);
		cvAnd(backproject, mask, backproject, 0);
		cvCopy(backproject, mask, NULL);
		findBlobs(newbt, mask);
		newbt->bbMinArea = newbt->blobs->area*0.4f;
		cvCamShift(backproject, *selection,
				   cvTermCriteria(CV_TERMCRIT_EPS|CV_TERMCRIT_ITER, 10, 1),
				   &trackcomp, 0);
		newbt->minArea = trackcomp.area*0.4f;
	}

	newbt->next = NULL;
	newbt->id = id;
	if (strlen(name) > 31)
		name[31] = '\0';
	strcpy(newbt->name, name);

	return newbt;
}

int loadBlobTrackers(BlobTracker **bt, char *filename)
{
	int i;
	float *bins;
	float hrangesArr[] = {0, 180};
	float* hranges = hrangesArr;
	FILE *fp;
	Blob *bb;
	BlobTracker tmpbt, *newbt, *prevbt=*bt;

	if (!(fp=fopen(filename, "rb")))
		return 0;
	while (fread(&tmpbt, sizeof(BlobTracker), 1, fp) > 0)
	{
		if (!(newbt=(BlobTracker *)malloc(sizeof(BlobTracker)+(sizeof(Blob)*tmpbt.numblobs)
										  +sizeof(CvHistogram)+(sizeof(float)*tmpbt.numbins))))
		{
			fclose(fp);
			return 0;
		}
		tmpbt.blobs = bb = (Blob *)(newbt+1);
		tmpbt.hist = (CvHistogram *)(bb+tmpbt.numblobs);
		bins = (float *)(tmpbt.hist+1);
		fread(bins, sizeof(float), tmpbt.numbins, fp);
		cvMakeHistHeaderForArray(1, &tmpbt.numbins, tmpbt.hist, bins, &hranges, 1);
		for(i=tmpbt.numblobs; i; --i, ++bb)
			bb->seenblob=bb->trackable=0;
		if (prevbt)
			prevbt->next = newbt;
		else
			*bt = newbt;
		*newbt = tmpbt;
		prevbt = newbt;
		newbt->next = NULL;
	}
	fclose(fp);
	return 1;
}

int saveBlobTrackers(BlobTracker *bt, char *filename)
{
	FILE *fp;

	if (!(fp=fopen(filename, "wb")))
		return 0;
	while (bt)
	{
		fwrite(bt, sizeof(BlobTracker), 1, fp);
		fwrite(cvGetHistValue_1D(bt->hist, 0), sizeof(float), 16, fp);
		bt = bt->next;
	}
	fclose(fp);
	return 1;
}

int initBlobTrackers(CvSize frameSize)
{
	hsv = cvCreateImage(frameSize, 8, 3);
	hue = cvCreateImage(frameSize, 8, 1);
	mask = cvCreateImage(frameSize, 8, 1);
	backproject = cvCreateImage(frameSize, 8, 1);
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.3, 1.3, 0.0, 1.3, 8);
	if (!(hsv && hue && mask && backproject))
	{
		closeBlobTrackers();
		return 0;
	}
	return 1;
}

void closeBlobTrackers()
{
	if (hsv)
		cvReleaseImage(&hsv);
	if (hue)
		cvReleaseImage(&hue);
	if (mask)
		cvReleaseImage(&mask);
	if (backproject)
		cvReleaseImage(&backproject);
}
