/*
 * Tencent is pleased to support the open source community by making
 * Hippy available.
 *
 * Copyright (C) 2022 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { HippyNativeModule } from '../../../../support/annotation/HippyNativeModule'
import { HippyException } from '../../../../support/common/HippyException'
import { HippyEngineContext } from '../../../HippyEngineContext'
import { HippyNativeModuleBase } from '../HippyNativeModuleBase'
import { audio } from '@kit.AudioKit'

@HippyNativeModule({ name: "AudioPlayerModule" })
export class AudioPlayerModule extends HippyNativeModuleBase {
  private mHippyEngineContext: HippyEngineContext
  private mMediaPlayer: audio.AudioRenderer | null

  constructor(ctx: HippyEngineContext) {
    super(ctx)
    this.mHippyEngineContext = ctx;
    this.mMediaPlayer = this.createMediaPlayer()
  }

  createMediaPlayer(): audio.AudioRenderer | null {
    let audioRendererOptions = null
    // let audioStreamInfo = {
    //   samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_44100,
    //   channels: audio.AudioChannel.CHANNEL_1,
    //   sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE,
    //   encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
    // }
    //
    // let audioRendererInfo = {
    //   content: audio.ContentType.CONTENT_TYPE_SPEECH,
    //   usage: audio.StreamUsage.STREAM_USAGE_VOICE_COMMUNICATION,
    //   rendererFlags: 0
    // }
    //
    // let audioRendererOptions = {
    //   streamInfo: audioStreamInfo,
    //   rendererInfo: audioRendererInfo
    // }

    audio.createAudioRenderer(audioRendererOptions).then((data) => {
      console.info('AudioFrameworkRenderLog: AudioRenderer Created : Success : Stream Type: SUCCESS');
      return data
    }).catch((err: HippyException) => {
      console.error(`AudioFrameworkRenderLog: AudioRenderer Created : ERROR : ${err}`);
    });
    return null
  }

  play() {
    if (this.mMediaPlayer != null) {
      this.mMediaPlayer.start().then(() => {
        console.info('Renderer started');
      }).catch((err:HippyException) => {
        console.error(`ERROR: ${err}`);
      });
    }
  }

  getState():string{
    if (this.mMediaPlayer != null) {
      return this.mMediaPlayer.state.toString()
    }
    return ""
  }

  goBack( seconds:number){

  }

  goForward( seconds:number){

  }

  seekTo( newPosition:number){

  }

  pause(){
    if (this.mMediaPlayer != null) {
      this.mMediaPlayer.pause().then(() => {
        console.info('Renderer pauseed');
      }).catch((err:HippyException) => {
        console.error(`ERROR: ${err}`);
      });
    }
  }

  resume(){

  }

  stop(){
    if (this.mMediaPlayer != null) {
      this.mMediaPlayer.stop().then(() => {
        console.info('Renderer stoped');
      }).catch((err:HippyException) => {
        console.error(`ERROR: ${err}`);
      });
    }
  }
}
