#!/usr/bin/env bun

import { access, readFile, writeFile } from 'node:fs/promises';
import path from 'node:path';
import { $ } from 'bun';

async function updatePackageVersion(version: string): Promise<void> {
  const packageJsonPath = path.join(__dirname, '..', 'package.json');

  try {
    await access(packageJsonPath);
  } catch {
    throw new Error(`package.json not found at ${packageJsonPath}`);
  }

  const packageJsonContent = await readFile(packageJsonPath, 'utf-8');
  const packageJson = JSON.parse(packageJsonContent);
  packageJson.version = version;

  await writeFile(
    packageJsonPath,
    `${JSON.stringify(packageJson, null, 2)}\n`,
    'utf-8',
  );
}

async function main(): Promise<void> {
  const version = (await $`git describe --tags --always`.text())
    .replace(/^v/, '')
    .trim();

  await updatePackageVersion(version);
  console.log(`package.json version set to ${version}`);
}

main().catch((error) => {
  console.error('prepublish failed:', error);
  process.exit(1);
});
