# 在 GitHub 启用 `main` 分支保护

仓库维护者请在 GitHub 网页配置（需 **Admin** 权限），与 [CONTRIBUTING.md](../CONTRIBUTING.md) 策略一致。

**路径**：`Settings` → `Branches` → `Add branch protection rule`

## 推荐规则

| 选项 | 建议 |
|------|------|
| Branch name pattern | `main` |
| **Require a pull request before merging** | ✅ 启用 |
| Required approvals | 0（单人项目）或 1+（团队） |
| **Require status checks to pass** | ✅ 启用 |
| Required checks | `ubuntu` / `macos`（或 workflow `build` 中的 job 名） |
| **Require branches to be up to date** | ✅ 建议启用 |
| **Do not allow bypassing** | ✅ 建议启用 |
| **Restrict who can push to matching branches** | 可选：仅维护者；或禁止所有人直推 |
| Allow force pushes | ❌ 关闭 |
| Allow deletions | ❌ 关闭 |

保存后，所有变更必须经过 PR，无法（或极难）直接 `git push origin main`。

## 验证

直推应被拒绝：

```bash
git checkout main
# 故意修改后
git push origin main
# 期望：remote rejected (protected branch hook declined)
```
