// Copyright 2020-present Nans Pellicari (nans.pellicari@gmail.com).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

class SWindowTimeline;

class FNansTimelineSystemToolbar
{
public:
	static void Initialize();

	static void Shutdown();

	void Register();
	void Unregister();

	void MyButton_Clicked() const;
	void AddToolbarExtension(FToolBarBuilder& builder) const;
	TSharedRef<SDockTab> MakeTimelineTab() const;
	TSharedRef<SWidget> GetWindowTimeline(const TSharedRef<SDockTab>& InParentTab) const;

private:
	static TSharedPtr<FNansTimelineSystemToolbar> Instance;
	TSharedPtr<FExtender> ToolbarExtender;
	TSharedPtr<const FExtensionBase> Extension;
};
