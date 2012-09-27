#include "db_database.h"
#include "db_pgn_reader.h"
#include "db_log.h"

#include "db_pgn_writer.h"
#include "u_zstream.h"

#include "u_progress.h"

#include "m_ifstream.h"

#include <jni.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <android/log.h>

#define  LOG_TAG    "NATIVECB"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


using namespace db;


static unsigned const g_Flags = Writer::Flag_Include_Variations
    | Writer::Flag_Include_Comments
    | Writer::Flag_Include_Annotation
    | Writer::Flag_Include_Marks
    | Writer::Flag_Include_Termination_Tag
    | Writer::Flag_Include_Mode_Tag
    | Writer::Flag_Include_Setup_Tag
    | Writer::Flag_Include_Variant_Tag
    | Writer::Flag_Include_Time_Mode_Tag
    | Writer::Flag_Exclude_Extra_Tags
    | Writer::Flag_Symbolic_Annotation_Style;

static unsigned rejected = 0;

struct Progress : public util::Progress
{
    void update(JNIEnv* env, jobject obj, jmethodID progressMid, unsigned progress) override
    {
        if (progressMid != 0) {
            env->CallVoidMethod(obj, progressMid, progress);
        }
    }

    void finish() throw() override
    {
    }
};

struct Log : public db::Log
{
    bool error(save::State code, unsigned gameNumber)
    {
        char const* msg = 0;

        switch (code)
        {
            case save::Ok:                              return true;
            case save::UnsupportedVariant:      printf("Unsupported Variant"); ++rejected; return true;
            case save::DecodingFailed:              printf("Decoding failed"); ++rejected; return true;
            case save::GameTooLong:                 msg = "Game too long"; break;
            case save::FileSizeExeeded:         msg = "File size exeeded"; break;
            case save::TooManyGames:                msg = "Too many games"; break;
            case save::TooManyPlayerNames:      msg = "Too many player names"; break;
            case save::TooManyEventNames:           msg = "Too many event names"; break;
            case save::TooManySiteNames:            msg = "Too many site names"; break;
            case save::TooManyRoundNames:           msg = "Too many round names"; break;
            case save::TooManyAnnotatorNames:   return true; // cannot happen
        }
        LOGE("%s: %s (#%u)\n",
                code == db::save::GameTooLong ? "Warning" : "Error",
                msg,
                gameNumber);
        return code == save::GameTooLong;
    }

    void warning(Warning code, unsigned gameNumber)
    {
        LOGW("warning: %u", code);
    }
};

static unsigned
exportGames(JNIEnv* env, jobject obj, jmethodID progressMid, Database& src, Consumer& dst, Progress& progress)
{
    unsigned numGames = src.countGames();
    LOGI("number of games: %u", numGames);
    jclass cls = env->GetObjectClass(obj);
    jmethodID mid = env->GetMethodID(cls, "setNOGames", "(I)V");
    if (mid != 0) {
        env->CallVoidMethod(obj, mid, numGames);
    }
    env->DeleteLocalRef(cls);

    util::ProgressWatcher watcher(progress, numGames);
    progress.setFrequency(mstl::min(5000u, mstl::max(numGames/100, 1u)));

    unsigned reportAfter = progress.frequency();
    unsigned count = 0;
    unsigned countGames = 0;

    ::Log log;

    for (unsigned i = 0; i < numGames; ++i)
    {
        if (reportAfter == count++)
        {
            progress.update(env, obj, progressMid, count);
            reportAfter += progress.frequency();
        }
        save::State state = src.exportGame(i, dst);

        if (state == save::Ok)
            ++countGames;
        else if (!log.error(state, i))
            return countGames;
    }

    return countGames;
}

/*
 * Class:     org_chess_cb_Cbh2PgnTask
 * Method:    convertToPgn
 */
extern "C" JNIEXPORT jint JNICALL Java_org_chess_cb_Cbh2PgnTask_convertToPgn
                (JNIEnv* env, jobject obj, jstring fileName, jstring outputDir)
{
    rejected = 0;
    int result = 0;
    const char* sourceFileName = (*env).GetStringUTFChars(fileName, NULL);
    const char* outDir;
    if (sourceFileName)
    {
        outDir = (*env).GetStringUTFChars(outputDir, NULL);
        if (outDir)
        {
            db::tag::initialize();
            db::castling::initialize();
            db::Board::initialize();
            db::HomePawns::initialize();
            db::Signature::initialize();

            mstl::string convertto("utf-8");
            mstl::string convertfrom("auto");

            mstl::string cbhPath(sourceFileName);
            if (cbhPath.size() < 4 || strcmp(cbhPath.c_str() + cbhPath.size() - 4, ".cbh") != 0)
                cbhPath.append(".cbh");
            LOGD("cbhpath=%s", cbhPath.c_str());
            mstl::ifstream  stream(cbhPath);

            mstl::string pgnPath;
            pgnPath.assign(cbhPath);
            mstl::string::size_type n = pgnPath.rfind('/');
            if (n != mstl::string::npos)
                pgnPath.erase(mstl::string::size_type(0), n + 1);
            if (pgnPath.size() < 4 || strcmp(pgnPath.c_str() + pgnPath.size() - 4, ".cbh") == 0)
                pgnPath.erase(pgnPath.size() - 4);
            if (pgnPath.size() < 4 || strcmp(pgnPath.c_str() + pgnPath.size() - 4, ".pgn") != 0)
                pgnPath.append(".pgn");
            mstl::string fullPgnPath;
            fullPgnPath.assign(outDir);
            fullPgnPath.append("/");
            fullPgnPath.append(pgnPath);

            jclass cls = env->GetObjectClass(obj);
            jmethodID mid = env->GetMethodID(cls, "setPgnFileName", "(Ljava/lang/String;)V");
            if (mid != 0) {
                env->CallVoidMethod(obj, mid, (*env).NewStringUTF(fullPgnPath.c_str()));
            }

            LOGI("initializing CB database...");
            Progress progress;
            Database src(cbhPath, convertfrom, Database::ReadOnly, progress);
            LOGI("finished initializing CB database");

            util::ZStream::Type type = util::ZStream::Text;
            mstl::ios_base::openmode mode = mstl::ios_base::out | mstl::ios_base::app;
            util::ZStream strm(fullPgnPath, type, mode);
            PgnWriter writer(format::Pgn, strm, convertto, g_Flags);

            LOGI("start exporting games to %s", fullPgnPath.c_str());
            jmethodID progressMid = env->GetMethodID(cls, "progress", "(I)V");
            unsigned numGames = exportGames(env, obj, progressMid, src, writer, progress);
            LOGI("%u game(s) written.", numGames);
            if (rejected > 0)
                LOGW("%u game(s) rejected.", rejected);
            fflush(stdout);
            src.close();
            result = numGames;
            env->DeleteLocalRef(cls);
        }
        (*env).ReleaseStringUTFChars(outputDir, outDir);
        (*env).ReleaseStringUTFChars(fileName, sourceFileName);
    }
    return result;
}
