/// <reference types="node" />

export type BinaryLike = Buffer | ArrayBuffer | ArrayBufferView;
export type ChunkCallback = (chunk: Buffer) => void;

export interface NativeAddon {
  diff(oldBuf: BinaryLike, newBuf: BinaryLike): Buffer;
  diff(oldBuf: BinaryLike, newBuf: BinaryLike, cb: ChunkCallback): void;
  compress(buf: BinaryLike): Buffer;
  compress(buf: BinaryLike, cb: ChunkCallback): void;
}

export const native: NativeAddon;

export function diff(oldBuf: BinaryLike, newBuf: BinaryLike): Buffer;

declare const bsdiff: {
  native: NativeAddon;
  diff: typeof diff;
};

export default bsdiff;
