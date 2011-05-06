/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef PROJECT_SV_H
#define PROJECT_SV_H

#include <QFile>
#include <QDir>
#include <QString>
#include <QImage>

#include <QObject>
#include <QFile>

class Flow_sV;
#include "../lib/defs_sV.h"

extern "C" {
#include "../lib/videoInfo_sV.h"
}

class RenderTask_sV;
class NodeList_sV;
class QSignalMapper;
class QProcess;
class QRegExp;
class QTimer;
class Project_sV : public QObject
{
    Q_OBJECT

public:
    Project_sV(QString filename, QString projectDir);
    ~Project_sV();

    const VideoInfoSV& videoInfo() const { return m_videoInfo; }
    Flow_sV *flow() const { return m_flow; }
    NodeList_sV *nodes() const { return m_nodes; }
    RenderTask_sV *renderTask() { return m_renderTask; }
    float fps() const { return m_fps; }

    enum FrameSize { FrameSize_Orig, FrameSize_Small };


    void render(qreal fps);


    /**
      @return true, iff all required directories exist
      */
    bool validDirectories() const;
    /**
      Extracts frames for all sizes.
      @param force Also generate frames if they already exist.
      */
    void extractFrames(bool force = false);
    /**
      Extracts the frames from the video file into single images
      */
    bool extractFramesFor(const FrameSize frameSize);
    /**
      Checks the availability of the frames and decides
      whether they need to be extracted with extractFrames()
      */
    bool rebuildRequired(const FrameSize frameSize) const;
    /**
      @return The frame at the given position, as image. Fails
      if the frames have not been extracted yet.
      */
    QImage frameAt(const uint frame, const FrameSize frameSize = FrameSize_Orig) const;

    QImage interpolateFrameAt(float time) const;

    QImage requestFlow(int leftFrame, FlowDirection direction, bool forceRebuild = false) const;


    const QString frameFileStr(int number) const;
    const QString thumbFileStr(int number) const;
    const QString flowFileStr(int leftFrame, FlowDirection direction) const;
    const QString renderedFileStr(int number) const;

public slots:
    void slotFlowCompleted();
    void slotSetFps(float fps);

signals:
    /**
      Emitted when all frames have been extracted for this frame size.
      */
    void signalFramesExtracted(Project_sV::FrameSize frameSize);
    /**
      Emitted when an ffmpeg thread has made progress (i.e. wrote to stderr).
      @param progress Number in the range 0...100
      */
    void signalProgressUpdated(Project_sV::FrameSize frameSize, int progress);


private:
    static QString defaultFramesDir;
    static QString defaultThumbFramesDir;
    static QString defaultFlowDir;
    static QString defaultRenderDir;
    static QRegExp regexFrameNumber;

    struct {
        bool origFinished;
        bool smallFinished;
        void reset() {
            origFinished = false;
            smallFinished = false;
        }
        bool allFinished() { return origFinished && smallFinished; }
    } m_processStatus;

    bool m_canWriteFrames;
    bool m_flowComplete;

    QFile m_inFile;
    QDir m_projDir;
    QDir m_framesDir;
    QDir m_thumbFramesDir;
    QDir m_flowDir;
    QDir m_renderDir;
    VideoInfoSV m_videoInfo;
    Flow_sV *m_flow;
    NodeList_sV *m_nodes;
    RenderTask_sV *m_renderTask;

    QSignalMapper *m_signalMapper;
    QProcess *m_ffmpegOrig;
    QProcess *m_ffmpegSmall;
    QTimer *m_timer;

    float m_fps;

    float timeToFrame(float time) const;



private slots:
    void slotExtractingFinished(int);
    /**
      Checks the progress of the ffmpeg threads by reading their stderr
      and emits signalProgressUpdated() if necessary.
      */
    void slotProgressUpdate();

};

QDebug operator<<(QDebug qd, const Project_sV::FrameSize &frameSize);

#endif // PROJECT_SV_H
