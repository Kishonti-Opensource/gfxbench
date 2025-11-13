/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.ByteBuffer;

import android.graphics.SurfaceTexture;
import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.util.Log;
import android.view.Surface;

// example class based mostly on ExtractMpegFramesTest.java sample Android test
// for more info see: http://bigflake.com/mediacodec/ExtractMpegFramesTest.java.txt
// and http://bigflake.com/mediacodec/
public class VideoFrameExtractor implements SurfaceTexture.OnFrameAvailableListener {
    private static final boolean VERBOSE = true;
    private static final String TAG = "VideoFrameExtractor";
    MediaCodec mDecoder = null;
    MediaExtractor mExtractor = null;
    private VideoStream mVideoStream;
    private int mVideoTrackIndex = -1;
    private boolean mInputDone = false;
    private boolean mOutputDone = false;
    int mDecodeCount = 0;
    private Object mFrameSyncObject = new Object();
    private boolean mFrameAvailable = false;
    private boolean mRestart = false;

    public VideoFrameExtractor(String filename) throws IOException {
        File inputFile = new File(filename); // must be an absolute path
        // The MediaExtractor error messages aren't very useful. Check to see if the input
        // file exists so we can throw a better one if it's not there.
        if (!inputFile.canRead()) {
            throw new FileNotFoundException("Unable to read " + inputFile);
        }

        mExtractor = new MediaExtractor();
        mExtractor.setDataSource(inputFile.toString());
        mVideoTrackIndex = selectTrack(mExtractor);
        if (mVideoTrackIndex < 0) {
            throw new RuntimeException("No video track found in " + inputFile);
        }
        mExtractor.selectTrack(mVideoTrackIndex);

        MediaFormat format = mExtractor.getTrackFormat(mVideoTrackIndex);
        int width = format.getInteger(MediaFormat.KEY_WIDTH);
        int height = format.getInteger(MediaFormat.KEY_HEIGHT);
        if (VERBOSE) Log.d(TAG, "Video size is " + width + "x" + height);

        // Could use width/height from the MediaFormat to get full-size frames.
        mVideoStream = new VideoStream();
        mVideoStream.setSize(width, height);
        mVideoStream.getSurfaceTexture().setOnFrameAvailableListener(this);

        // Create a MediaCodec decoder, and configure it with the MediaFormat from the
        // extractor. It's very important to use the format from the extractor because
        // it contains a copy of the CSD-0/CSD-1 codec-specific data chunks.
        String mime = format.getString(MediaFormat.KEY_MIME);
        mDecoder = MediaCodec.createDecoderByType(mime);
        Surface surface = new Surface(mVideoStream.getSurfaceTexture());
        mDecoder.configure(format, surface, null, 0);
        mDecoder.start();
    }

    public net.kishonti.swig.VideoStream getVideoStream() {
        return mVideoStream;
    }

    public void setEnableRestart(boolean restart) {
        mRestart = restart;
    }

    /**
     * Selects the video track, if any.
     *
     * @return the track index, or -1 if no video track is found.
     */
    private static int selectTrack(MediaExtractor extractor) {
        // Select the first video track we find, ignore the rest.
        int numTracks = extractor.getTrackCount();
        for (int i = 0; i < numTracks; i++) {
            MediaFormat format = extractor.getTrackFormat(i);
            String mime = format.getString(MediaFormat.KEY_MIME);
            if (mime.startsWith("video/")) {
                if (VERBOSE) Log.d(TAG, "Extractor selected track " + i + " (" + mime + "): " + format);
                return i;
            }
        }

        return -1;
    }

    /**
     * Work loop.
     */
    void extractFrame() throws IOException {
        if (mOutputDone)
            return;

        final int TIMEOUT_USEC = 10000;
        ByteBuffer[] decoderInputBuffers = mDecoder.getInputBuffers();
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        int inputChunk = 0;

        boolean frameExtracted = false;
        while (!frameExtracted) {
            if (VERBOSE) Log.d(TAG, "loop");

            // Feed more data to the decoder.
            if (!mInputDone) {
                int inputBufIndex = mDecoder.dequeueInputBuffer(TIMEOUT_USEC);
                if (inputBufIndex >= 0) {
                    ByteBuffer inputBuf = decoderInputBuffers[inputBufIndex];
                    // Read the sample data into the ByteBuffer. This neither respects nor
                    // updates inputBuf's position, limit, etc.
                    int chunkSize = mExtractor.readSampleData(inputBuf, 0);
                    if (chunkSize < 0) {
                        // End of stream -- send empty frame with EOS flag set.
                        mDecoder.queueInputBuffer(inputBufIndex, 0, 0, 0L, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                        mInputDone = true;
                        if (VERBOSE) Log.d(TAG, "sent input EOS");
                    } else {
                        if (mExtractor.getSampleTrackIndex() != mVideoTrackIndex) {
                            Log.w(TAG, "WEIRD: got sample from track " + mExtractor.getSampleTrackIndex() + ", expected " + mVideoTrackIndex);
                        }
                        long presentationTimeUs = mExtractor.getSampleTime();
                        mDecoder.queueInputBuffer(inputBufIndex, 0, chunkSize, presentationTimeUs, 0 /* flags */);
                        if (VERBOSE) Log.d(TAG, "submitted frame " + inputChunk + " to dec, size=" + chunkSize);
                        inputChunk++;
                        mExtractor.advance();
                    }
                } else {
                    if (VERBOSE) Log.d(TAG, "input buffer not available");
                }
            }

            if (!mOutputDone) {
                int decoderStatus = mDecoder.dequeueOutputBuffer(info, TIMEOUT_USEC);
                if (decoderStatus == MediaCodec.INFO_TRY_AGAIN_LATER) {
                    // no output available yet
                    if (VERBOSE) Log.d(TAG, "no output from decoder available");
                } else if (decoderStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                    // not important for us, since we're using Surface
                    if (VERBOSE) Log.d(TAG, "decoder output buffers changed");
                } else if (decoderStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                    MediaFormat newFormat = mDecoder.getOutputFormat();
                    if (VERBOSE) Log.d(TAG, "decoder output format changed: " + newFormat);
                } else if (decoderStatus < 0) {
                    throw new RuntimeException("unexpected result from decoder.dequeueOutputBuffer: " + decoderStatus);
                } else { // decoderStatus >= 0
                    if (VERBOSE) Log.d(TAG, "surface decoder given buffer " + decoderStatus + " (size=" + info.size + ")");
                    if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                        if (VERBOSE) Log.d(TAG, "output EOS");
                        mOutputDone = true;
                    }

                    boolean doRender = (info.size != 0);

                    // As soon as we call releaseOutputBuffer, the buffer will be forwarded
                    // to SurfaceTexture to convert to a texture. The API doesn't guarantee
                    // that the texture will be available before the call returns, so we
                    // need to wait for the onFrameAvailable callback to fire.
                    mDecoder.releaseOutputBuffer(decoderStatus, doRender);
                    if (doRender) {
                        if (VERBOSE) Log.d(TAG, "awaiting decode of frame " + mDecodeCount);
                        awaitNewImage();
                        mDecodeCount++;
                    }
                    frameExtracted = true;
                }
            }

            if (mRestart) {
                if (mOutputDone) {
                    mExtractor.seekTo(0, 0);
                    mDecoder.flush();
                    mOutputDone = false;
                    mInputDone = false;
                }
            }
        }

    }

    public void close() {
        mVideoStream = null;
        mExtractor = null;
        mDecoder.release();
        mDecoder = null;
    }

    /**
     * Latches the next buffer into the texture.  Must be called from the thread that created
     * the CodecOutputSurface object.  (More specifically, it must be called on the thread
     * with the EGLContext that contains the GL texture object used by SurfaceTexture.)
     */
    public void awaitNewImage() {
        final int TIMEOUT_MS = 2500;

        synchronized (mFrameSyncObject ) {
            while (!mFrameAvailable) {
                try {
                    // Wait for onFrameAvailable() to signal us.  Use a timeout to avoid
                    // stalling the test if it doesn't arrive.
                    mFrameSyncObject.wait(TIMEOUT_MS);
                    if (!mFrameAvailable) {
                        Log.w(TAG, "frame wait timed out");
                        break;
                    }
                } catch (InterruptedException ie) {
                    // shouldn't happen
                    throw new RuntimeException(ie);
                }
            }
            mFrameAvailable = false;
        }
    }

    // SurfaceTexture callback
    @Override
    public void onFrameAvailable(SurfaceTexture st) {
        if (VERBOSE) Log.d(TAG, "new frame available");
        synchronized (mFrameSyncObject) {
            if (mFrameAvailable) {
                Log.e(TAG, "mFrameAvailable already set, frame could be dropped");
            }
            mFrameAvailable = true;
            mFrameSyncObject.notifyAll();
        }
    }

    private class VideoStream extends net.kishonti.swig.VideoStream {
        private SurfaceTexture mSurfaceTexture;
        private net.kishonti.swig.FloatArray mCTexMatrix;
        private float[] mTexMatrix;

        public VideoStream() {
            super();
            int[] textures = new int[1];
            GLES20.glGenTextures(1, textures, 0);
            int texId = textures[0];
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, texId);
            GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
            GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
            mSurfaceTexture = new SurfaceTexture(texId);
            mTexMatrix = new float[16];
            mCTexMatrix = new net.kishonti.swig.FloatArray(16);
        }

        public SurfaceTexture getSurfaceTexture() {
            return mSurfaceTexture;
        }

        public void updateTexImage() {
            try {
                extractFrame();
                mSurfaceTexture.updateTexImage();
                mSurfaceTexture.getTransformMatrix(mTexMatrix);
                for (int i = 0; i < 16; ++i) { mCTexMatrix.setitem(i, mTexMatrix[i]); }
                setTransformMatrix(mCTexMatrix.cast());
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

    }

}
