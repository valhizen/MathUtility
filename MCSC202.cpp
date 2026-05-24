// MCSC202.cpp : Defines the entry point for the application.

#include "MCSC202.h"
#include <imgui.h>
#include <SDL3/SDL.h>
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include <SDL3/SDL_opengl.h>
#include "Lexer.hpp"
#include "Parser.hpp"

float epsilon = 0.0005f;
float tolerance = 0.001f;
int samples = 1000;

struct IterationData {
	int   iteration;
	float a, b, x0, fx0, interval_length;
};



static bool isContinuousAtPoint(std::function<float(float)> func, float point) {
	float left = func(point - epsilon);
	float right = func(point + epsilon);
	float center = func(point);


	bool left_match =
		std::abs(left - center) < tolerance;

	bool right_match =
		std::abs(right - center) < tolerance;

	return left_match && right_match;

}

static bool isContinuousAtInterval(std::function<float(float)> func, float point_a, float point_b)
{
	float step = (point_b - point_a) / samples;

	for (int i = 0; i <= samples; i++)
	{
		float x = point_a + i * step;

		if (!isContinuousAtPoint(func, x))
			return false;
	}
	return true;
}

static bool CheckIntermediateValueTherom(std::function<float(float)> func, float point_a, float point_b) {

	if (!isContinuousAtInterval(func, point_a, point_b)) {
		return false;
	}

	float val_1 = func(point_a);
	float val_2 = func(point_b);

	return { val_1 * val_2 <= 0 };
}

std::function<float(float)> function = nullptr;



static float findRoot(std::function<float(float)> func, float x1, float x2, std::vector<IterationData>* out_iterations = nullptr) {

	float point_1, point_2;
	if (func(x1) > 0) { point_1 = x2; point_2 = x1; }
	else { point_1 = x1; point_2 = x2; }

	float mid = 0.0f;
	for (int i = 0; i < 120000; i++) {
		mid = (point_1 + point_2) / 2.0f;
		float value = func(mid);

		if (out_iterations) {
			out_iterations->push_back({ i + 1, point_1, point_2, mid, value, std::abs(point_2 - point_1) });
		}

		if (std::abs(point_2 - point_1) < 0.0005f) break;

		if (value < 0) point_1 = mid;
		else           point_2 = mid;
	}

	return mid;
}

static void MCSC_Check(std::function<float(float) > func, char* x_start_value, char* x_end_value, char* game_value_x1, char* game_value_x2) {

	float x_start = strtof(x_start_value, nullptr);
	float x_end = strtof(x_end_value, nullptr);
	float continuous_check_x1 = strtof(game_value_x1, nullptr);
	float continuous_check_x2 = strtof(game_value_x2, nullptr);

	ImGui::Begin("MSCS CHECK WINDOW");

	float value_at_x1 = func(continuous_check_x1);
	float value_at_x2 = func(continuous_check_x2);

	float is_root_avilable = value_at_x1 * value_at_x2;

	if (is_root_avilable < 0) {
		ImGui::Text("Root Is in the Given Interval");
	}
	else {
		ImGui::Text("Root Is not in the Given Interval");
	}

	ImGui::Text("At Point %f, is %f", continuous_check_x1, value_at_x1);
	ImGui::Text("At Point %f, is %f", continuous_check_x2, value_at_x2);

	if (isContinuousAtInterval(func, continuous_check_x1, continuous_check_x2)) {
		ImGui::Text("Function is Continuous ");
	}
	else
	{
		ImGui::Text("Function is not Continuous ");
	}


	if (CheckIntermediateValueTherom(func, continuous_check_x1, continuous_check_x2) == true) {
		ImGui::Text("Function satisfies Immediate Value Theorem ");
	}
	else
	{
		ImGui::Text("Function does not satisfies Immediate Value Theorem");
	}

	std::vector<IterationData> iterations;
	float root = findRoot(func, continuous_check_x1, continuous_check_x2, &iterations);

	ImGui::Text("The Root of the Equation is %f", root);
	ImGui::Text("Converged after %d iterations", (int)iterations.size());

	// Iteration table
	ImGui::Spacing();
	if (ImGui::BeginTable("IterTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 300))) {

		ImGui::TableSetupColumn("Iter", ImGuiTableColumnFlags_WidthFixed, 40);
		ImGui::TableSetupColumn("a", ImGuiTableColumnFlags_WidthFixed, 110);
		ImGui::TableSetupColumn("b", ImGuiTableColumnFlags_WidthFixed, 110);
		ImGui::TableSetupColumn("x0", ImGuiTableColumnFlags_WidthFixed, 110);
		ImGui::TableSetupColumn("f(x0)", ImGuiTableColumnFlags_WidthFixed, 130);
		ImGui::TableSetupColumn("|b-a|", ImGuiTableColumnFlags_WidthFixed, 110);
		ImGui::TableHeadersRow();

		for (const IterationData& row : iterations) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn(); ImGui::Text("%d", row.iteration);
			ImGui::TableNextColumn(); ImGui::Text("%.6f", row.a);
			ImGui::TableNextColumn(); ImGui::Text("%.6f", row.b);
			ImGui::TableNextColumn(); ImGui::Text("%.6f", row.x0);
			ImGui::TableNextColumn(); ImGui::Text("%.6e", row.fx0);
			ImGui::TableNextColumn(); ImGui::Text("%.6f", row.interval_length);
		}

		ImGui::EndTable();
	}

	ImGui::End();
}

static void DrawFunctionGraph(std::function<float(float)> func, char* x_start_value, char* x_end_value, char* game_value_x1, char* game_value_x2) {

	float x_start = strtof(x_start_value, nullptr);
	float x_end = strtof(x_end_value, nullptr);

	float continuous_check_x1 = strtof(game_value_x1, nullptr);
	float continuous_check_x2 = strtof(game_value_x2, nullptr);

	float value_at_x1 = func(continuous_check_x1);
	float value_at_x2 = func(continuous_check_x2);
	float is_root_avilable = value_at_x1 * value_at_x2;
	float root = findRoot(func, continuous_check_x1, continuous_check_x2);

	if (x_start >= x_end) return;

	const float graph_width = 1000.0f;
	const float graph_height = 800.0f;
	const int   grid_lines = 10;

	float step = (x_end - x_start) / samples;
	float y_min = FLT_MAX, y_max = -FLT_MAX;

	std::vector<ImVec2> points;
	for (float x = x_start; x <= x_end; x += step) {
		float y = func(x);
		if (!std::isfinite(y)) continue;
		points.push_back(ImVec2(x, y));
		y_min = std::min(y_min, y);
		y_max = std::max(y_max, y);
	}

	if (y_min == y_max) { y_min -= 1.0f; y_max += 0.5f; }

	ImGui::Begin("Function Graph");
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 graph_tl = ImGui::GetCursorScreenPos();

	auto toScreen = [&](float x, float y) -> ImVec2 {
		float sx = graph_tl.x + (x - x_start) / (x_end - x_start) * graph_width;
		float sy = graph_tl.y + (1.0f - (y - y_min) / (y_max - y_min)) * graph_height;
		return ImVec2(sx, sy);
		};

	ImU32 grid_color = IM_COL32(80, 80, 80, 255);
	for (int i = 0; i <= grid_lines; i++) {
		float t = (float)i / grid_lines;

		float gx = graph_tl.x + t * graph_width;
		draw_list->AddLine(ImVec2(gx, graph_tl.y), ImVec2(gx, graph_tl.y + graph_height), grid_color, 1.0f);
		char x_label[16];
		snprintf(x_label, sizeof(x_label), "%.1f", x_start + t * (x_end - x_start));
		draw_list->AddText(ImVec2(gx - 10.0f, graph_tl.y + graph_height + 4.0f), IM_COL32(200, 200, 200, 255), x_label);

		float gy = graph_tl.y + t * graph_height;
		draw_list->AddLine(ImVec2(graph_tl.x, gy), ImVec2(graph_tl.x + graph_width, gy), grid_color, 1.0f);
		char y_label[16];
		snprintf(y_label, sizeof(y_label), "%.1f", y_max - t * (y_max - y_min));
		draw_list->AddText(ImVec2(graph_tl.x, gy - 7.0f), IM_COL32(200, 200, 200, 255), y_label);
	}

	draw_list->AddRect(graph_tl, ImVec2(graph_tl.x + graph_width, graph_tl.y + graph_height), IM_COL32(180, 180, 180, 255), 0.0f, 0, 1.5f);

	ImU32 axis_color = IM_COL32(200, 200, 200, 200);
	if (x_start <= 0.0f && x_end >= 0.0f)
		draw_list->AddLine(toScreen(0.0f, y_max), toScreen(0.0f, y_min), axis_color, 1.5f);
	if (y_min <= 0.0f && y_max >= 0.0f)
		draw_list->AddLine(toScreen(x_start, 0.0f), toScreen(x_end, 0.0f), axis_color, 1.5f);

	draw_list->AddCircle(toScreen(root, 0.0f), 2.0f, ImColor{ 0, 0, 255, 255 }, 10, 2.0f);

	std::vector<ImVec2> screen_points;
	screen_points.reserve(points.size());
	for (const ImVec2& p : points)
		screen_points.push_back(toScreen(p.x, p.y));
	draw_list->AddPolyline(screen_points.data(), (int)screen_points.size(), IM_COL32(255, 80, 80, 255), ImDrawFlags_None, 2.0f);

	ImGui::Dummy(ImVec2(graph_width, graph_height));
	ImGui::End();
}


static char input_buffer[128];

static char x_axis_start_buffer[128];
static char x_axis_end_buffer[128];
static char continuous_check_buffer_x1[128];
static char continuous_check_buffer_x2[128];





template <typename function_type>
static float solve_function(function_type value, std::function<float(float)> function, float point) {
	return function(point);
}


int main()
{


	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
	{
		printf("Error: SDL_Init(): %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

	SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	SDL_Window* window = SDL_CreateWindow("MCSC 202", (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
	if (window == nullptr)
	{
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return 1;
	}

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	if (gl_context == nullptr)
	{
		printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_ShowWindow(window);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)
	ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init();



	//Note : Sarbesh " Need to Do this Later"
	//ImGui_ImplOpenGL3_Init(glsl_version);

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	bool is_running = true;
	SDL_Event event;

	while (is_running) {
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL3_ProcessEvent(&event);

			if (event.type == SDL_EVENT_QUIT)
				is_running = false;
			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
				is_running = false;
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		ImGui::InputText("Take Function", input_buffer, IM_COUNTOF(input_buffer));
		ImGui::InputText("X_AXIS_START", x_axis_start_buffer, IM_COUNTOF(x_axis_start_buffer));
		ImGui::InputText("X_AXIS_END", x_axis_end_buffer, IM_COUNTOF(x_axis_end_buffer));

		ImGui::InputText("Check Contimuous x1", continuous_check_buffer_x1, IM_COUNTOF(continuous_check_buffer_x1));
		ImGui::InputText("Check Contimuous x2", continuous_check_buffer_x2, IM_COUNTOF(continuous_check_buffer_x2));


		if (ImGui::Button("Evaluate")) {
			Parser parse(input_buffer);
			function = parse.parse();
		}

		if (function) {
			DrawFunctionGraph(function, x_axis_start_buffer, x_axis_end_buffer, continuous_check_buffer_x1, continuous_check_buffer_x2);
			MCSC_Check(function, x_axis_start_buffer, x_axis_end_buffer, continuous_check_buffer_x1, continuous_check_buffer_x2);
		}
		ImGui::Render();
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();


	SDL_GL_DestroyContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

