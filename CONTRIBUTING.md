# 贡献指南

## 分支与合并策略（必须遵守）

**禁止直接向 `main` 提交或推送。** 所有变更必须通过 **功能分支 + Pull Request** 合并。

### 标准流程

```bash
# 1. 从最新 main 拉分支（使用 cursor/ 前缀）
git checkout main
git pull origin main
git checkout -b cursor/short-description-207b

# 2. 开发、测试
./scripts/build.sh --test

# 3. 提交并推送分支（不要 push main）
git add <files>
git commit -m "feat: concise message"
git push -u origin cursor/short-description-207b

# 4. 在 GitHub 创建 PR，目标分支为 main，等待 CI 通过后合并
gh pr create --base main --head cursor/short-description-207b --title "..." --body "..."
```

### 禁止事项

- 在 `main` 上直接 `git commit`
- `git push origin main`（除非你是维护者执行 **PR 合并后的同步**，且应由 GitHub Merge 按钮完成）
- 本地 `git merge feature` 到 `main` 后 `push main`（绕过 PR 审查与 CI 记录）

### 分支命名

- 前缀：`cursor/`
- 示例：`cursor/opencv-reader-207b`、`cursor/fix-build-script-207b`

### PR 要求

- 目标分支：`main`
- CI（`.github/workflows/build.yml`）必须通过
- 至少简要说明变更内容与测试方式
- 合并方式优先 **Squash merge** 或 **Merge commit**（由维护者选择），保持历史清晰

## 构建与测试

见 [docs/BUILD.md](docs/BUILD.md)。

## 仓库维护者：启用分支保护

在 GitHub 上限制直推 `main`，见 [.github/BRANCH_PROTECTION.md](.github/BRANCH_PROTECTION.md)。
