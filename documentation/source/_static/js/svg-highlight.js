/* support to highlight mermaid paths useful if you present a big image to others */
$(function () {
    let allPaths = $("path.path");
    let allArrows = $("path.arrowheadPath");

    allPaths.click(function (event) {
        if (!event.shiftKey) {
            allPaths.removeClass("svg-highlight");
            allArrows.removeClass("svg-highlight");
        }
        if ($(this).hasClass("svg-highlight")) {
            $(this).removeClass("svg-highlight");
            $(this).find("+ defs > marker > path").removeClass("svg-highlight");
        } else {
            $(this).addClass("svg-highlight");
            $(this).find("+ defs > marker > path").addClass("svg-highlight");
        }
    });

    allPaths.hover(
        function () {
            $(this).addClass("svg-highlight-hover");
            $(this).find("+ defs > marker > path").addClass("svg-highlight-hover");
        },
        function () {
            $(this).removeClass("svg-highlight-hover");
            $(this).find("+ defs > marker > path").removeClass("svg-highlight-hover");
        }
    );
});
